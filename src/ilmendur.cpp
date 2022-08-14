#include "ilmendur.hpp"
#include "buildconfig.hpp"
#include "texture_pool.hpp"
#include "map.hpp"
#include "player.hpp"
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
    if (SDL_CreateWindowAndRenderer(640, 480, SDL_WINDOW_OPENGL, &mp_window, &mp_renderer) < 0) {
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

int Ilmendur::run()
{
    using namespace std::chrono;

    // This loads all textures from the disk and uploads them to the
    // graphics card.
    mp_texture_pool = new TexturePool();

    Map m("Oak Fortress");

    Player p;
    p.warp(Vector2f(32, 32));
    p.turn(Actor::direction::left);

    bool run = true;

    high_resolution_clock::time_point start_time;
    milliseconds passed_time;
    while (run) {
        start_time = high_resolution_clock::now();

        bool keydowns[4];

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            switch (ev.type) {
            case SDL_QUIT:
                run = false;
                break;
            case SDL_KEYDOWN:
                switch (ev.key.keysym.sym) {
                case SDLK_UP:
                    keydowns[0] = true;
                    break;
                case SDLK_RIGHT:
                    keydowns[1] = true;
                    break;
                case SDLK_DOWN:
                    keydowns[2] = true;
                    break;
                case SDLK_LEFT:
                    keydowns[3] = true;
                    break;
                }
                if (keydowns[0] && !keydowns[1] && !keydowns[2] && !keydowns[3]) {
                    p.go(Player::godir::n);
                } else if (!keydowns[0] && keydowns[1] && !keydowns[2] && !keydowns[3]) {
                    p.go(Player::godir::e);
                } else if (!keydowns[0] && !keydowns[1] && keydowns[2] && !keydowns[3]) {
                    p.go(Player::godir::s);
                } else if (!keydowns[0] && !keydowns[1] && !keydowns[2] && keydowns[3]) {
                    p.go(Player::godir::w);
                } else if (keydowns[0] && keydowns[1] && !keydowns[2] && !keydowns[3]) {
                    p.go(Player::godir::ne);
                } else if (!keydowns[0] && keydowns[1] && keydowns[2] && !keydowns[3]) {
                    p.go(Player::godir::se);
                } else if (!keydowns[0] && !keydowns[1] && keydowns[2] && keydowns[3]) {
                    p.go(Player::godir::sw);
                } else if (keydowns[0] && !keydowns[1] && !keydowns[2] && keydowns[3]) {
                    p.go(Player::godir::nw);
                }
                break;
            case SDL_KEYUP:
                switch (ev.key.keysym.sym) {
                case SDLK_UP:
                    keydowns[0] = false;
                    break;
                case SDLK_RIGHT:
                    keydowns[1] = false;
                    break;
                case SDLK_DOWN:
                    keydowns[2] = false;
                    break;
                case SDLK_LEFT:
                    keydowns[3] = false;
                    break;
                default:
                    // Ignore
                    break;
                }
                if (p.isMoving()) {
                    if (keydowns[0] && !keydowns[1] && !keydowns[2] && !keydowns[3]) {
                        p.go(Player::godir::n);
                    } else if (!keydowns[0] && keydowns[1] && !keydowns[2] && !keydowns[3]) {
                        p.go(Player::godir::e);
                    } else if (!keydowns[0] && !keydowns[1] && keydowns[2] && !keydowns[3]) {
                        p.go(Player::godir::s);
                    } else if (!keydowns[0] && !keydowns[1] && !keydowns[2] && keydowns[3]) {
                        p.go(Player::godir::w);
                    } else if (keydowns[0] && keydowns[1] && !keydowns[2] && !keydowns[3]) {
                        p.go(Player::godir::ne);
                    } else if (!keydowns[0] && keydowns[1] && keydowns[2] && !keydowns[3]) {
                        p.go(Player::godir::se);
                    } else if (!keydowns[0] && !keydowns[1] && keydowns[2] && keydowns[3]) {
                        p.go(Player::godir::sw);
                    } else if (keydowns[0] && !keydowns[1] && !keydowns[2] && keydowns[3]) {
                        p.go(Player::godir::nw);
                    } else if (!keydowns[0] && !keydowns[1] && !keydowns[2] && !keydowns[3]) {
                        p.stopMoving();
                    }
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

        p.update();

        SDL_SetRenderDrawColor(mp_renderer, 0, 0, 0, 255);
        SDL_RenderClear(mp_renderer);
        m.draw(mp_renderer);
        p.draw(mp_renderer);
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
