#ifndef ILMENDUR_TEXTURE_POOL_HPP
#define ILMENDUR_TEXTURE_POOL_HPP
#include <string>
#include <map>
#include <SDL2/SDL.h>

struct TextureInfo
{
    SDL_Texture* p_texture;
    std::string name;
    int width;
    int height;
    int frames;
    int animation_time;
    int stridex;
    int stridey;
    int origx;
    int origy;
};

class TexturePool
{
public:
    TexturePool();
    ~TexturePool();

    TextureInfo* operator[](const std::string& name);
private:
    std::map<std::string,TextureInfo*> m_textures;
};

#endif /* ILMENDUR_TEXTURE_POOL_HPP */
