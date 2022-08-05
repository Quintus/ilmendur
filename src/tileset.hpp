#ifndef ILMENDUR_TILESET_HPP
#define ILMENDUR_TILESET_HPP
#include <string>
#include <filesystem>
#include <SDL2/SDL.h>

class Tileset
{
public:
    Tileset(const std::filesystem::path& filename);
    ~Tileset();

    SDL_Rect operator[](int lid) const;
    SDL_Texture* sdlTexture();
private:
    std::string m_name;
    int m_columns;
    int m_tilecount;
    SDL_Texture* mp_texid;
};

#endif /* ILMENDUR_TILESET_HPP */
