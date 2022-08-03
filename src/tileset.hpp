#ifndef ILMENDUR_TILESET_HPP
#define ILMENDUR_TILESET_HPP
#include <string>
#include <SDL2/SDL.h>

class Tileset
{
public:
    Tileset(const std::string& name);
    ~Tileset();
private:
    std::string m_name;
    int m_columns;
    SDL_Texture* mp_texid;
};

#endif /* ILMENDUR_TILESET_HPP */
