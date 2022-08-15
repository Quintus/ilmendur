#include "texture_pool.hpp"
#include "ilmendur.hpp"
#include "os.hpp"
#include "util.hpp"
#include "ini.h"
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <SDL2/SDL_image.h>

using namespace std;
namespace fs = std::filesystem;

static int iniHandler(void* ptr, const char* section, const char* name, const char* value)
{
    TextureInfo* p_texinfo = (TextureInfo*) ptr;

    if (strcmp(section, "object") == 0) {
        if (strcmp(name, "stridex") == 0) {
            p_texinfo->stridex = atoi(value);
        } else if (strcmp(name, "stridey") == 0) {
            p_texinfo->stridey = atoi(value);
        } else if (strcmp(name, "origx") == 0) {
            p_texinfo->origx = atoi(value);
        } else if (strcmp(name, "origy") == 0) {
            p_texinfo->origy = atoi(value);
        } else if (strcmp(name, "collx") == 0) {
            p_texinfo->collx = atoi(value);
        } else if (strcmp(name, "colly") == 0) {
            p_texinfo->colly = atoi(value);
        } else if (strcmp(name, "collw") == 0) {
            p_texinfo->collw = atoi(value);
        } else if (strcmp(name, "collh") == 0) {
            p_texinfo->collh = atoi(value);
        } else if (strcmp(name, "animated") == 0) {
            p_texinfo->animation_time = atoi(value);
        } else if (strcmp(name, "licensestr") == 0) {
            // Ignore this one
        } else {
            cerr << "INI warning: Ignoring unknown key `" << name << "' in section `" << section << "'" << endl;
        }
    } else {
        cerr << "INI error: unknown INI section `" << section << "'" << endl;
        return 0;
    }

    return 1;
}

static void parseIni(TextureInfo* p_texinfo, const fs::path& inipath)
{
    assert(fs::exists(inipath));

    // Set default values. Width and height properties are set by the caller.
    p_texinfo->frames         = 1;
    p_texinfo->stridex        = p_texinfo->width;
    p_texinfo->stridey        = p_texinfo->height;
    p_texinfo->animation_time = 0;
    p_texinfo->collx          = 0;
    p_texinfo->colly          = 0;
    p_texinfo->collw          = -1; // Default depends on stridex, which may be read from file
    p_texinfo->collh          = -1; // Default depends on stridex, which may be read from file
    p_texinfo->origx          = -1; // Default depends on stridex, which may be read from file
    p_texinfo->origy          = -1; // Default depends on stridey, which may be read from file

    ifstream file(inipath, ifstream::in);
    string iniraw(READ_FILE(file));

    int result = ini_parse_string(iniraw.c_str(), iniHandler, p_texinfo);
    if (result > 0) {
        string errmsg("INI syntax error in file `");
        errmsg += inipath.string();
        errmsg += "' at line ";
        errmsg += to_string(result);
        errmsg += "!";
        // In addition to aborting with an exception, emit this
        // message on standard error so that users experimenting with
        // their INI files see it.
        cerr << errmsg;
        throw(runtime_error(errmsg));
    } else if (result < 0) {
        throw(runtime_error(string("Internal inih error ") + to_string(result) + " while processing INI file `" + inipath.string() + "!"));
    }

    // Calculate defaults that depend on other values
    if (p_texinfo->origx == -1) {
        p_texinfo->origx = p_texinfo->stridex / 2;
    }
    if (p_texinfo->origy == -1) {
        p_texinfo->origy = p_texinfo->stridey / 2;
    }
    if (p_texinfo->collw == -1) {
        p_texinfo->collw = p_texinfo->stridex;
    }
    if (p_texinfo->collh == -1) {
        p_texinfo->collh = p_texinfo->stridey;
    }

    // Convenience calculation
    if (p_texinfo->stridex != p_texinfo->width) {
        p_texinfo->frames = p_texinfo->width / p_texinfo->stridex;
    }
}

TexturePool::TexturePool()
{
    int result = 0;
    for (const fs::directory_entry& iter: fs::directory_iterator(OS::gameDataDir() / fs::u8path("tilesets"))) {
        if (iter.path().extension() == fs::u8path(".png")) {
            ifstream file(iter.path(), ifstream::in | ifstream::binary);
            string binary(READ_FILE(file));
            assert(binary.size() > 1);

            TextureInfo* p_texinfo = new TextureInfo;
            p_texinfo->name = string("tilesets/") + iter.path().filename().u8string();
            p_texinfo->p_texture = IMG_LoadTexture_RW(Ilmendur::instance().sdlRenderer(), SDL_RWFromMem(binary.data(), binary.size()), 1);
            assert(p_texinfo->p_texture);

            result = SDL_QueryTexture(p_texinfo->p_texture, nullptr, nullptr, &p_texinfo->width, &p_texinfo->height);
            assert(result == 0);
            assert(p_texinfo->width > 0 && p_texinfo->height > 0);

            m_textures[p_texinfo->name] = p_texinfo;
        }
    }

    for (const fs::directory_entry& iter: fs::recursive_directory_iterator(OS::gameDataDir() / fs::u8path("gfx"))) {
        if (iter.path().extension() == fs::u8path(".png")) {
            ifstream file(iter.path(), ifstream::in | ifstream::binary);
            string binary(READ_FILE(file));
            assert(binary.size() > 1);

            TextureInfo* p_texinfo = new TextureInfo;
            p_texinfo->name = fs::relative(iter.path(), OS::gameDataDir() / fs::u8path("gfx")).u8string();
            p_texinfo->p_texture = IMG_LoadTexture_RW(Ilmendur::instance().sdlRenderer(), SDL_RWFromMem(binary.data(), binary.size()), 1);
            assert(p_texinfo->p_texture);

            result = SDL_QueryTexture(p_texinfo->p_texture, nullptr, nullptr, &p_texinfo->width, &p_texinfo->height);
            assert(result == 0);
            assert(p_texinfo->width > 0 && p_texinfo->height > 0);

            fs::path ini_path = iter.path().parent_path() / fs::u8path(iter.path().stem().u8string() + ".ini");
            if (fs::exists(ini_path)) {
                parseIni(p_texinfo, ini_path);
            }

            m_textures[p_texinfo->name] = p_texinfo;
        }
    }
}

TexturePool::~TexturePool()
{
    for(auto iter=m_textures.begin(); iter != m_textures.end(); iter++) {
        if (iter->second->p_texture) {
            SDL_DestroyTexture(iter->second->p_texture);
        }
        delete iter->second;
    }
}

/**
 * Access a texture and its metadata as it is stored in the texture
 * pool. `name` is a path, which is either of these:
 *
 * - A string starting with `tilesets/`, referring to a file in the
 *   `tilesets/` directory.
 * - Any other string, which is treated as a filename relative to the
 *   `gfx/` directory.
 *
 * The method returns an instance of TextureInfo. Access its `p_texture`
 * member to gain access to the actual SDL texture. The returned pointer
 * must not be freed; it is owned by TexturePool.
 */
TextureInfo* TexturePool::operator[](const std::string& name)
{
    return m_textures[name];
}
