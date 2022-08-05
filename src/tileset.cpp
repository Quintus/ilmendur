#include "tileset.hpp"
#include "ilmendur.hpp"
#include "os.hpp"
#include "util.hpp"
#include <fstream>
#include <utility>
#include <cassert>
#include <pugixml.hpp>
#include <SDL2/SDL_image.h>

#define TILEWIDTH 32

using namespace std;
namespace fs = std::filesystem;

Tileset::Tileset(const fs::path& filename)
    : m_columns(0),
      m_tilecount(0),
      mp_texid(nullptr)
{
    fs::path abs_path(OS::gameDataDir() / fs::u8path("tilesets") / filename);
    fstream file(abs_path);
    assert(fs::exists(abs_path));

    pugi::xml_document doc;
    if (!doc.load(file)) {
        throw(std::runtime_error(string("Failed to load tileset file '") + abs_path.c_str() + "'"));
    }

    if (doc.child("tileset").attribute("version").value() != string("1.5")) {
        throw(std::runtime_error(string("Expected TSX tileset format version 1.5, got '") + doc.child("tileset").attribute("version").value() + "'."));
    }
    if (doc.child("tileset").attribute("tilewidth").as_int() != TILEWIDTH) {
        throw(std::runtime_error(string("Tileset '" + m_name + "' does not have " + to_string(TILEWIDTH) + "px tile width")));
    }
    if (doc.child("tileset").attribute("tileheight").as_int() != TILEWIDTH) {
        throw(std::runtime_error(string("Tileset '" + m_name + "' does not have " + to_string(TILEWIDTH) + "px tile height")));
    }

    m_columns   = doc.child("tileset").attribute("columns").as_int();
    m_tilecount = doc.child("tileset").attribute("tilecount").as_int();

    // Only TILEWIDTHxTILEWIDTH tilesets are supported
    assert(TILEWIDTH == doc.child("tileset").attribute("tilewidth").as_int());
    assert(TILEWIDTH == doc.child("tileset").attribute("tileheight").as_int());
    file.close();

    // Read the actual image file from disk (assumes the same directory as the TSX file),
    // and upload it to the graphics card.
    string imgpath = doc.child("tileset").child("image").attribute("source").value();
    abs_path       = OS::gameDataDir() / fs::u8path("tilesets") / fs::u8path(imgpath);
    file           = fstream(abs_path, fstream::in | fstream::binary);

    assert(fs::exists(abs_path));
    std::string str(READ_FILE(file));
    mp_texid = IMG_LoadTexture_RW(Ilmendur::instance().sdlRenderer(), SDL_RWFromMem(str.data(), str.size()), 1);
}

Tileset::~Tileset()
{
    if (mp_texid) {
        SDL_DestroyTexture(mp_texid);
    }
}

/**
 * Returns an SDL_Rect describing the region from the tileset texture
 * that corresponds to the Tiled local tile ID `lid`.
 */
SDL_Rect Tileset::operator[](int lid) const
{
    assert(lid < m_tilecount);
    return move(SDL_Rect{lid % m_columns * TILEWIDTH,
                         lid / m_columns * TILEWIDTH,
                         TILEWIDTH,
                         TILEWIDTH});
}

/**
 * Retrieves the SDL_Texture corresponding to this tileset.
 * Do not delete or modify this pointer; the memory is owned
 * by this class.
 */
SDL_Texture* Tileset::sdlTexture()
{
    return mp_texid;
}
