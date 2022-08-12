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
        } else if (strcmp(name, "licensestr")) {
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
    p_texinfo->origx          = p_texinfo->width / 2;
    p_texinfo->origy          = p_texinfo->height / 2;

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

    for (const fs::directory_entry& iter: fs::directory_iterator(OS::gameDataDir() / fs::u8path("gfx"))) {
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

TextureInfo* TexturePool::operator[](const std::string& name)
{
    return m_textures[name];
}
