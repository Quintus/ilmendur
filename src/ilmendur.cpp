#include "ilmendur.hpp"
#include "buildconfig.hpp"
#include "texture_pool.hpp"
#include "map.hpp"
#include "player.hpp"
#include "scene.hpp"
#include <chrono>
#include <thread>
#include <stdexcept>
#include <cassert>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#ifdef ILMENDUR_DEBUG_BUILD
#include <iostream>
#endif

using namespace std;

/* Window dimensions. Do not occupy the full HD space by default,
 * so that window decorations do not cause the window to become
 * larger than the screen. Fullscreen mode uses the entire screen
 * space. */
#define NORMAL_WINDOW_WIDTH 1910
#define NORMAL_WINDOW_HEIGHT 1020

// The maximum length of one frame, calculated from the desired frame rate.
const chrono::milliseconds TARGET_FRAMETIME{1000 / ILMENDUR_TARGET_FRAMERATE};

static Ilmendur* sp_ilmendur = nullptr;

Ilmendur::Ilmendur()
    : mp_window(nullptr),
      mp_renderer(nullptr),
      mp_texture_pool(nullptr)
{
    if (sp_ilmendur) {
        throw(runtime_error("Ilmendur is a singleton!"));
    }
    sp_ilmendur = this;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        throw(runtime_error("SDL_Init() failed!"));
    }
    if (IMG_Init(IMG_INIT_PNG) < 0) {
        throw(runtime_error("IMG_Init() failed!"));
    }

    // TODO: add flag SDL_WINDOW_ALLOW_HIGHDPI
    if (SDL_CreateWindowAndRenderer(NORMAL_WINDOW_WIDTH, NORMAL_WINDOW_HEIGHT, SDL_WINDOW_OPENGL, &mp_window, &mp_renderer) < 0) {
        throw(runtime_error("SDL_CreateWindowAndRenderer() failed!"));
    }

    assert(mp_window);
    assert(mp_renderer);

#ifdef ILMENDUR_DEBUG_BUILD
    SDL_RendererInfo ri;
    SDL_GetRendererInfo(mp_renderer, &ri);
    cout << "Renderer information: " << endl
         << "    Name:             " << ri.name << endl
         << "    Supported flags:  " << ri.flags << endl
         << "    Max texture size: " << ri.max_texture_width << "x" << ri.max_texture_height << endl;
#endif
}

Ilmendur::~Ilmendur()
{
    if (mp_texture_pool) {
        delete mp_texture_pool;
    }

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

/**
 * The part of the rendering area describing player 1.
 */
SDL_Rect Ilmendur::viewportPlayer1() const
{
    int width = 0;
    int height = 0;
    SDL_GetRendererOutputSize(mp_renderer, &width, &height);
    return SDL_Rect{0,0,width/2-1,height}; // -1 for a small divider space
}

/**
 * The part of the rendering area describing player 2.
 */
SDL_Rect Ilmendur::viewportPlayer2() const
{
    int width = 0;
    int height = 0;
    SDL_GetRendererOutputSize(mp_renderer, &width, &height);
    return SDL_Rect{width/2+1,0,width/2-1,height};
}

int Ilmendur::run()
{
    using namespace std::chrono;

    // This loads all textures from the disk and uploads them to the
    // graphics card.
    mp_texture_pool = new TexturePool();

    Scene testscene;

    Player* p = new Player(testscene);
    p->warp(Vector2f(1600, 2600));
    p->turn(Actor::direction::left);

    bool run = true;

    high_resolution_clock::time_point start_time;
    milliseconds passed_time;
    while (run) {
        start_time = high_resolution_clock::now();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                run = false;
                break;
            case SDL_KEYDOWN:
                switch (ev.key.keysym.sym) {
                case SDLK_UP:
                case SDLK_RIGHT: // fall-through
                case SDLK_DOWN:  // fall-through
                case SDLK_LEFT:  // fall-through
                    p->checkInput();
                    break;
                default:
                    // Ignore
                    break;
                }
                break;
            case SDL_KEYUP:
                switch (ev.key.keysym.sym) {
                case SDLK_UP:
                case SDLK_RIGHT: // fall-through
                case SDLK_DOWN:  // fall-through
                case SDLK_LEFT:  // fall-through
                    p->checkInput();
                    break;
                case SDLK_ESCAPE:
                    run = false;
                    break;
                default:
                    // Ignore
                    break;
                }
                break;
            default:
                // Ignore
                break;
            }

            if (ev.type == SDL_QUIT) {
                run = false;
            }
        }

        testscene.update();

        SDL_SetRenderDrawColor(mp_renderer, 0, 0, 0, 255);
        SDL_RenderClear(mp_renderer);
        testscene.draw(mp_renderer);
        SDL_RenderPresent(mp_renderer);

        // Throttle framerate to a fixed one (fixed frame rate)
        passed_time = duration_cast<milliseconds>(high_resolution_clock::now() - start_time);
        if (passed_time < TARGET_FRAMETIME) {
            this_thread::sleep_for(TARGET_FRAMETIME - passed_time);
        }
#ifdef ILMENDUR_DEBUG_BUILD
        else {
            cout << "Warning: Framerate below " << ILMENDUR_TARGET_FRAMERATE << "!" << endl;
        }
#endif
    }

    return 0;
}
