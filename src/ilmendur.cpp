#include "ilmendur.hpp"
#include <stdexcept>
#include <cassert>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

static Ilmendur* sp_ilmendur = nullptr;

Ilmendur::Ilmendur()
    : mp_window(nullptr),
      mp_renderer(nullptr)
{
    if (sp_ilmendur) {
        throw(std::runtime_error("Ilmendur is a singleton!"));
    }
    sp_ilmendur = this;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw(std::runtime_error("SDL_Init() failed!"));
    }
    if (IMG_Init(IMG_INIT_PNG) < 0) {
        throw(std::runtime_error("IMG_Init() failed!"));
    }

    if (SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI, &mp_window, &mp_renderer) < 0) {
        throw(std::runtime_error("SDL_CreateWindowAndRenderer() failed!"));
    }

    assert(mp_window);
    assert(mp_renderer);
}

Ilmendur::~Ilmendur()
{
    SDL_DestroyRenderer(mp_renderer);
    SDL_DestroyWindow(mp_window);

    IMG_Quit();
    SDL_Quit();
    sp_ilmendur = nullptr;
}

Ilmendur& Ilmendur::instance()
{
    return *sp_ilmendur;
}

int Ilmendur::run()
{
    SDL_SetRenderDrawColor(mp_renderer, 255, 0, 0, 255);
    bool run = true;
    while (run) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_QUIT) {
                run = false;
            }
        }

        SDL_RenderClear(mp_renderer);
        SDL_RenderPresent(mp_renderer);
    }

    return 0;
}
