#include "ilmendur.hpp"
#include "buildconfig.hpp"
#include "texture_pool.hpp"
#include "map.hpp"
#include "actors/player.hpp"
#include "scene.hpp"
#include "audio.hpp"
#include "os.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"
#include <chrono>
#include <thread>
#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <cassert>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

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
      mp_texture_pool(nullptr),
      mp_audio_system(nullptr),
      mp_testscene(nullptr)
{
    if (sp_ilmendur) {
        throw(runtime_error("Ilmendur is a singleton!"));
    }
    sp_ilmendur = this;

    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        throw(runtime_error(string("SDL_Init() failed: ") + SDL_GetError()));
    }
    if (IMG_Init(IMG_INIT_PNG) < 0) {
        throw(runtime_error(string("IMG_Init() failed: ") + SDL_GetError()));
    }
    if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
        throw(runtime_error(string("Mix_Init() failed: ") + SDL_GetError()));
    }
    assert(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096) == 0);

    // TODO: add flag SDL_WINDOW_ALLOW_HIGHDPI
    if (SDL_CreateWindowAndRenderer(NORMAL_WINDOW_WIDTH, NORMAL_WINDOW_HEIGHT, SDL_WINDOW_OPENGL, &mp_window, &mp_renderer) < 0) {
        throw(runtime_error(string("SDL_CreateWindowAndRenderer() failed: ") + SDL_GetError()));
    }

    assert(mp_window);
    assert(mp_renderer);

    ImGui::CreateContext();
    ImGui_ImplSDL2_InitForSDLRenderer(mp_window, mp_renderer);
    ImGui_ImplSDLRenderer_Init(mp_renderer);

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
    delete mp_testscene;

    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    if (mp_audio_system) {
        delete mp_audio_system;
    }

    if (mp_texture_pool) {
        delete mp_texture_pool;
    }

    SDL_DestroyRenderer(mp_renderer);
    SDL_DestroyWindow(mp_window);

    Mix_Quit();
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

static void loadFont(ImGuiIO& io)
{
    namespace fs = std::filesystem;
    fs::path fontpath(OS::gameDataDir() / fs::u8path("fonts") / fs::u8path("LinLibertine_R.otf"));
    ifstream file(fontpath, ifstream::in | ifstream::binary);
    string binary(READ_FILE(file));

    // Hand over to ImGui, which takes ownership of `buf'.
    char* buf = new char[binary.size()];
    memcpy(buf, binary.data(), binary.size());
    assert(io.Fonts->AddFontFromMemoryTTF(buf, binary.size(), 22.0f));
}

int Ilmendur::run()
{
    using namespace std::chrono;

    mp_texture_pool = new TexturePool();
    mp_audio_system = new AudioSystem();

    // TODO: Implement a proper scene stack.
    mp_testscene = new Scene();

    Player* p = new Player();
    p->warp(Vector2f(1600, 2600));
    p->turn(Actor::direction::left);
    mp_testscene->map().addActor(p, "chars");
    mp_testscene->setPlayer(p);

    ImGuiIO& io = ImGui::GetIO();
    bool run = true;

    loadFont(io);

    high_resolution_clock::time_point start_time;
    milliseconds passed_time;
    while (run) {
        start_time = high_resolution_clock::now();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL2_ProcessEvent(&ev);

            switch (ev.type) {
            case SDL_QUIT:
                run = false;
                break;
            case SDL_KEYDOWN:
                // Ignore event if ImGui has focus
                if (io.WantCaptureKeyboard) {
                    continue;
                }

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

        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui::NewFrame();

        mp_testscene->update();
        ImGui::Text("Hello, world!");

        SDL_RenderSetViewport(mp_renderer, nullptr);
        SDL_RenderSetClipRect(mp_renderer, nullptr);
        SDL_SetRenderDrawColor(mp_renderer, 0, 0, 0, 255);
        SDL_RenderClear(mp_renderer);
        mp_testscene->draw(mp_renderer);
        ImGui::Render();
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
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

Scene& Ilmendur::currentScene()
{
    return *mp_testscene;
}
