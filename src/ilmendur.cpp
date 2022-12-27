#include "ilmendur.hpp"
#include "buildconfig.hpp"
#include "texture_pool.hpp"
#include "map.hpp"
#include "actors/player.hpp"
#include "scenes/scene.hpp"
#include "scenes/title_scene.hpp"
#include "audio.hpp"
#include "gui.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_sdlrenderer.h"
#include <chrono>
#include <thread>
#include <stdexcept>
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
      mp_next_scene(nullptr),
      m_pop_scene(false)
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

    m_render_area.x = 0;
    m_render_area.y = 0;
    SDL_GetRendererOutputSize(mp_renderer, &m_render_area.w, &m_render_area.h);

#ifdef ILMENDUR_DEBUG_BUILD
    SDL_RendererInfo ri;
    SDL_GetRendererInfo(mp_renderer, &ri);

    cout << "Renderer information: " << endl
         << "    Name:             " << ri.name << endl
         << "    Supported flags:  " << ri.flags << endl
         << "    Max texture size: " << ri.max_texture_width << "x" << ri.max_texture_height << endl
         << "    Rendering area:   " << m_render_area.w << "x" << m_render_area.h << endl;

    int mix_freq;
    Uint16 mix_format;
    int mix_chans;
    if (Mix_QuerySpec(&mix_freq, &mix_format, &mix_chans)) {
        cout << "Audio device information: " << endl
             << "    Frequency: " << mix_freq << " Hz" << endl
             << "    Format: " << mix_format << endl
             << "    Channels: " << mix_chans << endl;
    } else {
        cerr << "Failed to open audio device" << endl;
    }
#endif
}

Ilmendur::~Ilmendur()
{
    while (!m_scene_stack.empty()) {
        Scene* p_scene = m_scene_stack.top();
        delete p_scene;
        m_scene_stack.pop();
    }

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

const SDL_Rect& Ilmendur::renderArea() const
{
    return m_render_area;
}

/**
 * The part of the rendering area describing player 1.
 */
SDL_Rect Ilmendur::viewportPlayer1() const
{
    return SDL_Rect{0,0,m_render_area.w/2-1,m_render_area.h}; // -1 for a small divider space
}

/**
 * The part of the rendering area describing player 2.
 */
SDL_Rect Ilmendur::viewportPlayer2() const
{
    return SDL_Rect{m_render_area.w/2+1,0,m_render_area.w/2-1,m_render_area.h};
}

int Ilmendur::run()
{
    using namespace std::chrono;

    mp_texture_pool = new TexturePool();
    mp_audio_system = new AudioSystem();

    m_scene_stack.push(new TitleScene());

    GUISystem::loadFonts();

    ImGuiIO& io = ImGui::GetIO();
    high_resolution_clock::time_point start_time;
    milliseconds passed_time;
    bool run = true;
    while (run) {
        start_time = high_resolution_clock::now();

        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            ImGui_ImplSDL2_ProcessEvent(&ev);
            if (GUISystem::handleEvent(ev)) {
                continue;
            }

            switch (ev.type) {
            case SDL_QUIT:
                run = false;
                break;
            case SDL_KEYDOWN:
                // Ignore event if ImGui has focus
                if (!io.WantCaptureKeyboard) {
                    m_scene_stack.top()->handleKeyDown(ev);
                }
                break;
            case SDL_KEYUP:
                // Ignore event if ImGui has focus
                if (!io.WantCaptureKeyboard) {
                    m_scene_stack.top()->handleKeyUp(ev);
                }
                break;
            default:
                // Ignore
                break;
            }
        }

        ImGui_ImplSDL2_NewFrame();
        ImGui_ImplSDLRenderer_NewFrame();
        ImGui::NewFrame();

        m_scene_stack.top()->update();
        GUISystem::update();

        SDL_RenderSetViewport(mp_renderer, nullptr);
        SDL_RenderSetClipRect(mp_renderer, nullptr);
        SDL_SetRenderDrawColor(mp_renderer, 0, 0, 0, 255);
        SDL_RenderClear(mp_renderer);
        m_scene_stack.top()->draw(mp_renderer);
        ImGui::Render();
        ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
        SDL_RenderPresent(mp_renderer);

        if (m_pop_scene) {
            Scene* p_scene = m_scene_stack.top();
            m_scene_stack.pop();
            m_pop_scene = false;
            delete p_scene;

            if (m_scene_stack.empty() && !mp_next_scene) {
                run = false;
            }
        }
        if (mp_next_scene) {
            m_scene_stack.push(mp_next_scene);
            mp_next_scene = nullptr;
        }

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
    return *m_scene_stack.top();
}

/**
 * At the end of the frame, pop the current topmost scene
 * from the stack. To have a new one there, call pushScene()
 * after calling this. Otherwise the scene below the current
 * topmost scene will become active again (which might be intended).
 * Popping the last scene from the stack initiates game termination.
 */
void Ilmendur::popScene()
{
    m_pop_scene = true;
}

/**
 * Push a new scene onto the scene stack at the end of the frame.
 * In order to remove the current scene, be sure to call popScene()
 * before this. Otherwise the scenes will stack (which might be intended).
 */
void Ilmendur::pushScene(Scene* p_scene)
{
    mp_next_scene = p_scene;
}
