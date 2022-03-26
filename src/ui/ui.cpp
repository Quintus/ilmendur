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

struct GUIEngine::FontData {
    nk_user_font nkfont;
    GLuint atlas_texid;
    map<unsigned long,FT_Glyph_Metrics> glyph_metrics;
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
        const FT_Glyph_Metrics& metrics = p_ftdata->glyph_metrics[static_cast<unsigned long>(cp)]; // cp can only ever be positive, but ICU uses a signed type for whatever reason
        result += metrics.horiAdvance >> 6;
    }

    return result;
}

static void fontglyphcb(nk_handle handle, float font_height, struct nk_user_font_glyph *glyph, nk_rune codepoint, nk_rune next_codepoint)
{
    GUIEngine::FontData* p_ftdata = static_cast<GUIEngine::FontData*>(handle.ptr);

    const FT_Glyph_Metrics& metrics = p_ftdata->glyph_metrics[static_cast<unsigned long>(codepoint)];
    glyph->width = metrics.width >> 6;
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
    : mp_ft(new FontData())
{
    buildFontAtlas();

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
    //cfg.null = dev->null;
}
