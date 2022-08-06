#ifndef ILMENDUR_TILESET_HPP
#define ILMENDUR_TILESET_HPP
#include <string>
#include <filesystem>
#include <SDL2/SDL.h>

/**
 * Class representing a tileset. This object is not copyable --
 * it contains an SDL_Texture, which refers to memory on the
 * graphics card.
 *
 * TODO: Actually delete the copy constructor
 */
class Tileset
{
public:
    Tileset(const std::filesystem::path& filename);
    ~Tileset();

    void readTile(SDL_Rect& rect, int lid) const;
    SDL_Texture* sdlTexture();
private:
    std::string m_name;
    int m_columns;
    int m_tilecount;
    SDL_Texture* mp_texid;
};

#endif /* ILMENDUR_TILESET_HPP */
