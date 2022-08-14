#ifndef ILMENDUR_TEXTURE_POOL_HPP
#define ILMENDUR_TEXTURE_POOL_HPP
#include <string>
#include <map>
#include <SDL2/SDL.h>

/**
 * An information store on textures. Apart from the
 * texture itself, which is available in the `p_texture`
 * member, this object encapsulates all the metadata
 * that has been parsed from the INI files accompanying
 * each graphic.
 */
struct TextureInfo
{
    SDL_Texture* p_texture; ///< Underlying SDL texture
    std::string name;       ///< Name of this texture in the texture pool
    int width;              ///< Width in pixels
    int height;             ///< Height in pixels
    int frames;             ///< For animated graphics: number of frames (1 otherwise)
    int animation_time;     ///< For animated graphics: how many milliseconds to display each frame in a looped (= non-move) animation (0 otherwise)
    int stridex;            ///< For animated graphics: width of one frame (equals `width` otherwise)
    int stridey;            ///< For animated graphics: height of one frame (equals `height` otherwise)
    int origx;              ///< Origin X coördinate (defaults to 1/2 of `stridex` if unset in INI)
    int origy;              ///< Origin Y coördinate (defaults to 1/2 of `stridey` if unset in INI)
};

/**
 * This class manages all textures in the game. There should only
 * be one instance of it ever used, and it can be accesed through
 * the Ilmendur singleton.
 *
 * Constructing this class loads all graphics files from disk and
 * uploads them to the graphics card, making them textures. These
 * textures, along with the metadata stored in the corresponding
 * INI files, is available through the [] operator.
 *
 */
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
