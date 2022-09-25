#ifndef ILMENDUR_ILMENDUR_HPP
#define ILMENDUR_ILMENDUR_HPP
#include <SDL2/SDL.h>

/// Target framerate in frames per second (fps).
const unsigned int ILMENDUR_TARGET_FRAMERATE = 40;

class TexturePool;
class AudioSystem;
class Scene;
class DebugMapScene;

class Ilmendur
{
public:
    Ilmendur();
    ~Ilmendur();

    static Ilmendur& instance();

    int run();

    inline SDL_Window*   sdlWindow()   { return mp_window; }
    inline SDL_Renderer* sdlRenderer() { return mp_renderer; }
    inline TexturePool&  texturePool() { return *mp_texture_pool; }
    inline AudioSystem&  audioSystem() { return *mp_audio_system; }
           Scene&        currentScene();

    SDL_Rect viewportPlayer1() const;
    SDL_Rect viewportPlayer2() const;

private:
    void playAudio();
    SDL_Window*   mp_window;
    SDL_Renderer* mp_renderer;
    TexturePool*  mp_texture_pool;
    AudioSystem*  mp_audio_system;
    DebugMapScene* mp_testscene;
};

#endif /* ILMENDUR_ILMENDUR_HPP */
