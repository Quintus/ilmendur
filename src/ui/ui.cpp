#include "ui.hpp"
#include "../os/paths.hpp"
#include "nuklear.h"
#include <fstream>
#include <iostream>
#include <iterator>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cstdint>
#include <cassert>

#include <GL/gl3w.h> /* Note: gl3w is included in Ogre3D */
#include <OgreBitwise.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <unicode/unistr.h>
#include <unicode/schriter.h>

using namespace std;
using namespace UISystem;
namespace fs = std::filesystem;

static const int FONTSIZE = 30; // Desired font size in pixels
static const int CELLS_PER_ROW = 32; // How many cells should the font atlas have per row?

// Vertex shader for the Nuklear UI library
static const char* s_nkvertexshader = R"glsl(
#version 150
in vec2 position;
in vec2 texcoords;
in vec4 color;
out vec2 TexCoords;
out vec4 Color;

void main()
{
    TexCoords = texcoords;
    Color = color;
    gl_Position = vec4(position.x, position.y, 0, 1);
})glsl";

// Fragment shader for the Nuklear UI library
static const char* s_nkfragmentshader = R"glsl(
#version 150
uniform sampler2D Texture;
in vec2 TexCoords;
in vec4 Color;
out vec4 outColor;

void main()
{
    outColor = Color * texture(Texture, TexCoords.st);
}
)glsl";

struct GUIEngine::FontData {
    nk_user_font nkfont;
    GLuint atlas_texid;
    GLuint null_texid;
    map<unsigned long,FT_Glyph_Metrics> glyph_metrics;
};

struct GUIEngine::Vertex {
    float x;
    float y;
    float s;
    float t;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct GUIEngine::OpenGLData {
    GLuint shaderprog_id; ///< Holds the OpenGL ID for the compiled shader programme
    GLuint vao_id;        ///< Holds the OpenGL ID for the Vertex Array Object used for the UI
    GLuint vbo_id;        ///< OpenGL ID for the vertex array on the graphics card; the actual vertices are refilled each frame from what Nuklear UI provides!
    GLuint ebo_id;        ///< OpenGL ID for the element array buffer corresponding to `vbo_id'.

    GLuint position_loc;  ///< GPU memory location of the `position' input of the compiled shader programme `shaderprog_id'
    GLuint texcoords_loc; ///< GPU memory location of the `texcoords' input of the compiled shader programme `shaderprog_id'
    GLuint color_loc;     ///< GPU memory location of the `color' input of the compiled shader programme `shaderprog_id'

    GLuint texid_loc;      ///< GPU memory location of the `Texture' uniform of the compiled shader programme `shaderprog_id'
};

static float textwidthcb(nk_handle handle, float height, const char* text, int len)
{
    GUIEngine::FontData* p_ftdata = static_cast<GUIEngine::FontData*>(handle.ptr);

    // First, decompose the string into its code points
    const icu::UnicodeString utf16text = icu::UnicodeString::fromUTF8(icu::StringPiece(text, len));

    // Then, iterate each code point as if we were rendering it,
    // measuring all advances on the X axis for each code point.
    float result = 0.0f;
    icu::StringCharacterIterator iter(utf16text);
    for(UChar32 cp=iter.first32(); cp != icu::StringCharacterIterator::DONE; cp=iter.next32()) {
        assert(p_ftdata->glyph_metrics.count(static_cast<unsigned long>(cp)) != 0);
        const FT_Glyph_Metrics& metrics = p_ftdata->glyph_metrics[static_cast<unsigned long>(cp)]; // cp can only ever be positive, but ICU uses a signed type for whatever reason
        result += metrics.horiAdvance >> 6;
    }

    return result;
}

static void fontglyphcb(nk_handle handle, float font_height, struct nk_user_font_glyph *glyph, nk_rune codepoint, nk_rune next_codepoint)
{
    GUIEngine::FontData* p_ftdata = static_cast<GUIEngine::FontData*>(handle.ptr);

    assert(p_ftdata->glyph_metrics.count(static_cast<unsigned long>(codepoint)) != 0);
    const FT_Glyph_Metrics& metrics = p_ftdata->glyph_metrics[static_cast<unsigned long>(codepoint)];
    glyph->width = metrics.width >> 6; // ">> 6" for converting from Freetype's 26.6 fractional pixel format
    glyph->height = metrics.height >> 6;
    glyph->xadvance = metrics.horiAdvance >> 6;
    // TODO: Measure properly
    glyph->uv[0].x = 0;
    glyph->uv[1].y = 0;
    glyph->uv[1].x = 32;
    glyph->uv[1].y = 32;
    glyph->offset.x = metrics.horiBearingX;
    glyph->offset.y = metrics.horiBearingY;
}

GUIEngine::GUIEngine()
    : mp_ft(new FontData()),
      mp_ogl(new OpenGLData())
{
    buildFontAtlas();
    makeNullTexture();
    compileShaders();

    m_nkfont.userdata.ptr = mp_ft;
    m_nkfont.height = FONTSIZE;
    m_nkfont.width = textwidthcb;
    m_nkfont.query = fontglyphcb;
    m_nkfont.texture.id = mp_ft->atlas_texid;

    nk_init_default(&m_nkcontext, &m_nkfont);
}

GUIEngine::~GUIEngine()
{
    nk_free(&m_nkcontext);
    delete mp_ft;
    delete mp_ogl;
}

void GUIEngine::buildFontAtlas()
{
    std::string fontfilename = "LinLibertine_R.otf";
    // Step 1: Initialise Freetype, load the font file into memory, and
    // request a sensible font size.
    fs::path fontpath = OS::game_resource_dir() / fs::u8path("fonts") / fs::u8path(fontfilename);
    ifstream fontfile(fontpath, ios::in | ios::binary);
    string fontfiledata(istreambuf_iterator<char>{fontfile}, {}); // Read all of fontfile into str

    FT_Library p_ftlib;
    FT_Face p_ftface;
    if (FT_Init_FreeType(&p_ftlib)) {
        throw(std::runtime_error("Error initialising freetype!"));
    }

    if (FT_New_Memory_Face(p_ftlib, reinterpret_cast<FT_Byte*>(fontfiledata.data()), fontfiledata.size(), 0, &p_ftface)) {
        throw(std::runtime_error("Error loading the font file with freetype!"));
    }

    // FIXME: Should better use FT_Set_Char_Size() and calculate
    // the proper relative pixel font size from the current DPI.
    // Note: The pixel value calculated here must be equal to what
    // is passed to s_nkfont.height further below.
    if (FT_Set_Pixel_Sizes(p_ftface, 0, FONTSIZE)) {
        throw(std::runtime_error("Failed to set freetype font size!"));
    }

    // Step 2: Render a sensible set of glyphs into a font atlas for
    // use by nuklear.
    // FIXME: For now only does ASCII (Basic Latin Unicode block)
    // Step 2a: Determine the global font metrics.

    /* Retrieve the maximum bounding box for this font which can hold any glyph from it.
     * p_ftface->bbox is in *font units* as per Freetype documentation, which means that
     * is a constant value relative to a font size of 12 pt. The current font scale
     * (as requested above) is available via p_ftface->size->metrics and in theory
     * simply needs to be multiplied with the base values for 12 pt. The font scale
     * however is in super-precise Freetype 16.16 fractional format which cannot directly
     * be multiplied. Freetype provides a special function FT_MulFix(), which allows
     * to multiply font units with 16.16 format values. The result is in 1/64th of
     * pixels (Freetype 26.6 fractional format), which can be converted to the actual
     * pixel values by rshifting with 6, see <https://freetype.org/freetype2/docs/tutorial/step1.html>.
     * As the Freetype docs warn that the result may be slightly too small due to hinting,
     * add another pixel for extra security to have enough space. */
    const long maxglyphwidth  = (FT_MulFix(labs(p_ftface->bbox.xMin) + labs(p_ftface->bbox.xMax), p_ftface->size->metrics.y_scale)>>6) + 1;
    const long maxglyphheight = (FT_MulFix(labs(p_ftface->bbox.yMin) + labs(p_ftface->bbox.yMax), p_ftface->size->metrics.y_scale)>>6) + 1;

    /* Determine where the font's baseline is. The baseline sits the amount of
     * the font's `descender' above each cell's lower edge, that is, it is
     * at the same position for each glyph. Descenders go below it, but in
     * their utmost extend they exactly only touch the lower edge.
     * `descender' is in Freetype font units, i.e. 1/64th of a pixel. As
     * per Freetype docs, rshifting it by 6 gives the amount in pixels.
     * Also note that `descender' is *negative* so it needs to be added
     * instead of subtracted to the cell's height. */
    const size_t baseline = maxglyphheight + (p_ftface->descender >> 6);

    const long num_glyphs = 0x0250; // 0x024f is the last char in latin-b extended, should suffice for most western languages
    /* Characters outside the range 0x0-0x024f will not render. That's
     * basically a restriction of the nuklear UI library, because it
     * only works with font atlases. Freetype can render anything, but
     * using it properly would mean to call Freetype's functions each
     * time a glyph is requested, which nuklear UI simply can't. It's
     * possible to render a larger font atlas by incrementing the
     * above value for `num_glyphs', though, but beware that already
     * rendering the Basic Multilingual Plane results in a huge
     * texture (>3 GiB with the Linux Libertine font), and that's just
     * a fraction of Unicode. */

    // Step 2b: Create a 2-dimensional font atlas image in memory.

    // Calculate total expanse of the font atlas in pixels. Graphics card drivers generally
    // require textures to be a power of 2, so cater for that as well.
    const size_t atlaswidth  = Ogre::Bitwise::firstPO2From(maxglyphwidth * CELLS_PER_ROW); // The font atlas will encompass CELLS_PER_ROW glyphs in width, the rest needs to be done with the height
    const size_t atlasheight = Ogre::Bitwise::firstPO2From((num_glyphs / CELLS_PER_ROW + 1) * maxglyphheight); // How many rows we need to fit all glyphs; +1 to cater for divisions with remainder

    /* Create 2d pixel array for the font atlas, zero out the memory. In this atlas,
     * a value of 0 means to make a pixel transparent, and 255 means to make it all black.
     * This matches the representation used by Freetype. After this code completes, the font
     * atlas is all transparent. */
    unsigned char** fontatlas = new unsigned char*[atlaswidth];
    for(size_t i=0; i<atlaswidth; i++) {
        fontatlas[i] = new unsigned char[atlasheight];
        memset(fontatlas[i], 0, atlasheight);
    }

    // For each glyph of the chosen glyph set, make an entry in the font atlas.
    size_t targetcell = 0;
    for (unsigned long c=0x0; c < num_glyphs; c++, targetcell++) {
        // Get, glyph index, load glyph image into the font slot, convert to bitmap
        if (FT_Load_Char(p_ftface, c, FT_LOAD_RENDER)) {
            cout << "Warning: Failed to render char: " << c << endl;
            continue;
        }

        /* If this triggers, the font file is errorneous, because it
         * sets the max bounding box incorrectly. Or the bbox calculation
         * further above is incorrect, but I really hope it is not, because
         * I spent quite a lot of time reading Freetype docs to get it right. */
        assert(p_ftface->glyph->bitmap.rows <= maxglyphheight);

        // Remember this glyph's metrics (required for calculating a string's width
        // for nuklear UI library).
        mp_ft->glyph_metrics.emplace(make_pair(c, p_ftface->glyph->metrics));

        // Blit the bitmap onto the font atlas image
        // Note that the target cell in the atlas image is most likely
        // larger, because it is calculated for the maximum glyph extent!
        // In FT_PIXEL_MODE_GRAY mode, Freetype stores 1 pixel in 1 byte,
        // where the exact value stored gives the grayness of the pixel
        // as a value between 0 and 255. Other colours do not exist in
        // FT_PIXEL_MODE_GRAY mode.
        size_t cell_start_x = (targetcell % CELLS_PER_ROW) * maxglyphwidth;
        size_t cell_start_y = (targetcell / CELLS_PER_ROW) * maxglyphheight;
        assert(p_ftface->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY);

        size_t penx = cell_start_x;
        size_t peny = cell_start_y;

        // Reset Y pen to the baseline, then go to the glyph's vertical
        // starting position above the baseline (`bitmap_top').
        peny = cell_start_y + baseline;
        peny -= p_ftface->glyph->bitmap_top;
        for (unsigned int row = 0; row < p_ftface->glyph->bitmap.rows; row++) {

            /* Position the glyph in the cell's middle. `bitmap_left` is usually zero,
             * but it can be quite negative in cases where the glyph is intended to
             * expand below the previous glyph in a text (e.g. cursive "f").
             * The glyph width is in 1/64th of a pixel, thus rshift it by 6
             * as per the freetype docs to get a pixel value. */
            penx += maxglyphwidth/2 - (p_ftface->glyph->metrics.width>>6) / 2
                 + p_ftface->glyph->bitmap_left;

            // DEBUG
            //if (p_ftface->glyph->bitmap_left != 0) {
            //    printf("!!!!!!!Glyph %ld has a nonzero bitmap-left: %d\n", c, p_ftface->glyph->bitmap_left);
            //}

            for(unsigned int x=0; x < p_ftface->glyph->bitmap.width; x++) {
                unsigned char pixel = p_ftface->glyph->bitmap.buffer[row * p_ftface->glyph->bitmap.pitch + x];
                fontatlas[penx][peny] = pixel;
                //                      Note ↑: Freetype gives (in `pixel') 0 for transparent and 255 for black
                penx++;
            }

            penx = cell_start_x;
            peny++;
        }
    }

    ofstream bmpfile("/tmp/test.pnm", ios::out | ios::binary);
    bmpfile << "P2" << " " << to_string(atlaswidth) << " " << atlasheight << " " << "255" << "\n";

    for(size_t y=0; y < atlasheight; y++) {
        for(size_t x=0; x < atlaswidth; x++) {
            unsigned char pixel = fontatlas[x][y];
            //size_t targetx = cell_start_x + x * 4;
            //size_t targety = cell_start_y + row * 4;

            if (255-pixel < 100)
                bmpfile << " ";
            if (255-pixel < 10)
                bmpfile << " ";
            bmpfile << " ";
            bmpfile << to_string(255-pixel);

            //atlas_pixels[targetx+targety+0] = pixel; // R
            //atlas_pixels[targetx+targety+1] = pixel; // G
            //atlas_pixels[targetx+targety+2] = pixel; // B
            //atlas_pixels[targetx+targety+3] = 0xBB;  // A
        }
        bmpfile << "\n";
    }

    // Step 3: Convert the entire font atlas into an RGB array for OpenGL.
    // Note: Freetype gives black as 255, whereas OpenGL wants it as 0.
    unsigned char* rawpixels = new unsigned char[atlaswidth * atlasheight * 3]; // 3 = 1 value for each of R, G, B
    unsigned char* ptr = rawpixels;
    for(size_t y=0; y < atlasheight; y++) {
        for(size_t x=0; x < atlaswidth; x++) {
            *ptr++ = 255 - fontatlas[x][y]; // R
            *ptr++ = 255 - fontatlas[x][y]; // G
            *ptr++ = 255 - fontatlas[x][y]; // B
        }
    }

    // Upload font atlas to graphics card
    glGenTextures(1, &mp_ft->atlas_texid);
    glBindTexture(GL_TEXTURE_2D, mp_ft->atlas_texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, atlaswidth, atlasheight, 0, GL_RGB, GL_UNSIGNED_BYTE, rawpixels);

    // Final step: Clean up all resources.
    for(size_t i=0; i<atlaswidth; i++) {
        delete[] fontatlas[i];
    }
    delete[] fontatlas;
    delete[] rawpixels;

    FT_Done_Face(p_ftface);
    FT_Done_FreeType(p_ftlib);
    //delete[] atlas_pixels;
}

void GUIEngine::makeNullTexture()
{
    glGenTextures(1, &mp_ft->null_texid);
    glBindTexture(GL_TEXTURE_2D, mp_ft->atlas_texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const float whitepixel[] = {1.0f, 1.0f, 1.0f};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT, whitepixel);
}

void GUIEngine::compileShaders()
{
    // Step 1: Compile vertex and fragment shader
    GLint status = GL_FALSE;
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &s_nkvertexshader, NULL);
    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char log[1024];
        glGetShaderInfoLog(vertex_shader, 1024, NULL, log);
        throw(runtime_error(string("Vertex shader compilation failed: ") + log));
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &s_nkfragmentshader, NULL);
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char log[1024];
        glGetShaderInfoLog(fragment_shader, 1024, NULL, log);
        throw(runtime_error(string("Fragment shader compilation failed: ") + log));
    }

    // Step 2: Link them into a graphics card programme
    mp_ogl->shaderprog_id = glCreateProgram();
    glAttachShader(mp_ogl->shaderprog_id, vertex_shader);
    glAttachShader(mp_ogl->shaderprog_id, fragment_shader);
    glLinkProgram(mp_ogl->shaderprog_id);
    glGetProgramiv(mp_ogl->shaderprog_id, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char log[1024];
        glGetProgramInfoLog(mp_ogl->shaderprog_id, 1024, NULL, log);
        throw(runtime_error(string("Shader programme failed to link: ") + log));
    }

    // TODO: Can one just delete those here? Keeping the programme around
    // should suffice.
    //glDeleteShader(vertex_shader);
    //glDeleteShader(fragment_shader);

    /* Step 3: Create an informational object (VAO) that describes how
     * to transform the raw vertices into input for the vertex shader.
     * The format of the vertex array is static, thus we canjust add
     * this description here even though we don't have actual vertex
     * arrays yet; they will later be provided for each frame by
     * Nuklear UI (see uploadVertices()). However, as we don't have
     * the actual vertex arrays yet, the VBO and EBO remain empty at
     * this point. This is however sufficient for glVertexAttribPointer().
     * See GUIEngine::Vertex for the layout of the vertices. */
    glGenBuffers(1, &mp_ogl->vbo_id);
    glGenBuffers(1, &mp_ogl->ebo_id);
    glGenVertexArrays(1, &mp_ogl->vao_id);

    // Retrieve locations of all inputs for the vertex shader. Note how
    // this corresponds to all `in' entries of the vertex shader.
    // Storing them in `mp_ogl' optimises out the call to glGetAttribLocation()
    // for each frame which would be necessary otherwise in draw().
    mp_ogl->position_loc  = glGetAttribLocation(mp_ogl->shaderprog_id, "position");
    mp_ogl->texcoords_loc = glGetAttribLocation(mp_ogl->shaderprog_id, "texcoords");
    mp_ogl->color_loc     = glGetAttribLocation(mp_ogl->shaderprog_id, "color");

    // Now do the same for the shaders' uniforms.
    mp_ogl->texid_loc      = glGetUniformLocation(mp_ogl->shaderprog_id, "Texture");

    // Create the VAO
    glBindVertexArray(mp_ogl->vao_id);
    glBindBuffer(GL_ARRAY_BUFFER, mp_ogl->vbo_id);
    glVertexAttribPointer(mp_ogl->position_loc, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float)+4*sizeof(unsigned char), nullptr);
    glVertexAttribPointer(mp_ogl->texcoords_loc, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float)+4*sizeof(unsigned char), reinterpret_cast<void*>(2*sizeof(float)));
    glVertexAttribPointer(mp_ogl->color_loc, 2, GL_UNSIGNED_BYTE, GL_TRUE, 4*sizeof(float)+4*sizeof(unsigned char), reinterpret_cast<void*>(4*sizeof(float)));
    glEnableVertexAttribArray(mp_ogl->position_loc);
    glEnableVertexAttribArray(mp_ogl->texcoords_loc);
    glEnableVertexAttribArray(mp_ogl->color_loc);

    // Leave clean state
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void GUIEngine::draw()
{
    nk_convert_config cfg;
    memset(&cfg, 0, sizeof(nk_convert_config));

    nk_draw_vertex_layout_element vertex_layout[] = {
        {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, 0},
        {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, sizeof(float)*2},
        {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, sizeof(float)*4},
        {NK_VERTEX_LAYOUT_END}
    };

    cfg.shape_AA = NK_ANTI_ALIASING_ON;
    cfg.line_AA = NK_ANTI_ALIASING_ON;
    cfg.vertex_layout = vertex_layout;
    cfg.vertex_size = sizeof(float)*8;
    cfg.vertex_alignment = 0;
    cfg.circle_segment_count = 22;
    cfg.curve_segment_count = 22;
    cfg.arc_segment_count = 22;
    cfg.global_alpha = 1.0f;
    cfg.null.texture.id = mp_ft->null_texid;
    cfg.null.uv.x = 0;
    cfg.null.uv.y = 0;

    // TODO: Move into constructor and re-use the buffers
    nk_buffer_init_default(&m_nkcmds);
    nk_buffer_init_default(&m_nkvertices);
    nk_buffer_init_default(&m_nkelements);

    // Nuklear UI requires some undocumented global OpenGL state changes.
    // These can be found by looking at the demos provided with it.
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);

    // Switch to our nuklear UI shader programme.
    glUseProgram(mp_ogl->shaderprog_id);
    glUniform1i(mp_ogl->texid_loc, 0); // Set to NULL texture at the beginning

    // TODO: Move cfg creation into constructor
    uploadVertices(&cfg);

    const nk_draw_command* p_cmd = nullptr;
    unsigned int elementno = 0;
    nk_draw_foreach(p_cmd, &m_nkcontext, &m_nkcmds) {
        if (!p_cmd->elem_count) {
            continue;
        }

        // Copy the configuration from nuklear UI to OpenGL
        glBindTexture(GL_TEXTURE_2D, static_cast<GLuint>(p_cmd->texture.id));
        //glScissor(static_cast<GLint>(cmd->clip_rect.x),
        //          static_cast<GLint>(cmd->clip_rect.y),
        //          static_cast<GLint>(cmd->clip_rect.w),
        //          static_cast<GLint>(cmd_clip_rect.h));

        // Draw it!
        glDrawElements(GL_TRIANGLES, p_cmd->elem_count, GL_UNSIGNED_INT, reinterpret_cast<void*>(elementno));
        elementno += p_cmd->elem_count;
    };

    // Leave clean state (corresponding binds happened in uploadVertices()),
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);

    nk_clear(&m_nkcontext);
    nk_buffer_clear(&m_nkcmds);
    nk_buffer_clear(&m_nkvertices);
    nk_buffer_clear(&m_nkelements);
    // TODO: Move into destructor and re-use the buffers
    nk_buffer_free(&m_nkelements);
    nk_buffer_free(&m_nkvertices);
    nk_buffer_free(&m_nkcmds);
}

void GUIEngine::uploadVertices(nk_convert_config* p_cfg)
{
    /* Instruct Nuklear UI to output draw commands for this frame
     * into `m_nkcmds'. All required vertices will be output to
     * `m_nkvertices' and corresponding vertex indices to `m_nkelements'.
     * Each command in `m_nkcmds' then contains both some configuration
     * for the draw command (e.g., active texture) and the number of
     * indices to consume from `m_nkelements'. The vertex format
     * in m_nkvertices' is dictated by `p_cfg->vertex_layout'. */
    nk_convert(&m_nkcontext, &m_nkcmds, &m_nkvertices, &m_nkelements, p_cfg);

    // We will now upload the vertex array contained in `mk_vertices' to
    // the graphics card, along with the element array contained in
    // `m_nkelements'. Because we draw this once and then throw it away,
    // request GL_STREAM_DRAW memory.
    glBindVertexArray(mp_ogl->vao_id);
    glBindBuffer(GL_ARRAY_BUFFER,         mp_ogl->vbo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mp_ogl->ebo_id);
    glBufferData(GL_ARRAY_BUFFER,         m_nkvertices.memory.size, m_nkvertices.memory.ptr, GL_STREAM_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_nkelements.memory.size, m_nkelements.memory.ptr, GL_STREAM_DRAW);
}
