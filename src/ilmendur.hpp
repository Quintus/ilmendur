#ifndef ILMENDUR_ILMENDUR_HPP
#define ILMENDUR_ILMENDUR_HPP
#include <SDL2/SDL.h>

/// Target framerate in frames per second (fps).
const unsigned int ILMENDUR_TARGET_FRAMERATE = 40;

class Ilmendur
{
public:
    Ilmendur();
    ~Ilmendur();

    static Ilmendur& instance();

    int run();

    inline SDL_Window*   sdlWindow()   { return mp_window; }
    inline SDL_Renderer* sdlRenderer() { return mp_renderer; }

private:
    SDL_Window*   mp_window;
    SDL_Renderer* mp_renderer;
};

#endif /* ILMENDUR_ILMENDUR_HPP */
