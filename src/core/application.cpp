#include "application.hpp"
#include "resolver.hpp"
#include "window.hpp"
#include "game_state.hpp"
#include "../os/paths.hpp"
#include "../scenes/dummy_scene.hpp"
#include "../scenes/joymenu_scene.hpp"
#include "../ui/ui.hpp"
#include <buildconfig.hpp>
#include <GLFW/glfw3.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OgreGL3PlusPlugin.h>
#include <OgreParticleFXPlugin.h>
#include <OgreSTBICodec.h>
#include <OgreDotSceneLoader.h>
#include <iostream>
#include <chrono>
#include <unistd.h>

// Remove this once OpenSUSE shops GCC >= 8
#if defined(__GNUC__) && __GNUC__ < 8
#include <experimental/filesystem>
namespace std {
    namespace filesystem = std::experimental::filesystem;
}
#else
#include <filesystem>
#endif

namespace fs = std::filesystem;
using namespace std;
using namespace Core;

static Application* sp_application = nullptr;

static void processGLFWKeys(GLFWwindow* p_glfw_window,
                            int key, int scancode, int action, int mods)
{
    Application::getSingleton()
        .currentScene()
        .processKeyInput(key, scancode, action, mods);
}

static void processGLFWChars(GLFWwindow* p_glfw_window,
                             unsigned int codepoint)
{
    Application::getSingleton()
        .currentScene()
        .processCharInput(codepoint);
}

static void processGLFWCursorMove(GLFWwindow* p_glfw_window,
                                  double xpos, double ypos)
{
    Application::getSingleton()
        .currentScene()
        .processCursorMove(xpos, ypos);
}

static void processGLFWMouseButton(GLFWwindow* p_glfw_window,
                                   int button, int action, int mods)
{
    Application::getSingleton()
        .currentScene()
        .processMouseButton(button, action, mods);
}

Application::Application()
    : m_fps(0.0f),
      mp_window(nullptr),
      mp_sglistener(nullptr)
{
    assert(!sp_application); // There can only be one application instance
    //locale::global(locale(""));
    const char* locname = setlocale(LC_ALL, "");
    assert(locname);

    // Setup Internationalisation (i18n) support
    const char* tbasedir = bindtextdomain(ILMENDUR_GETTEXT_DOMAIN, OS::translations_dir().u8string().c_str());
    assert(tbasedir);

    const char* tcharset = bind_textdomain_codeset(ILMENDUR_GETTEXT_DOMAIN, "UTF-8");
    assert(tcharset);

    const char* tdomain = textdomain(ILMENDUR_GETTEXT_DOMAIN);
    assert(tdomain);

    cout << "  ( )   ___    __     __ _____ _   ___ _____   ___  ___ ____" << endl
         << " <   >  | |    | \\   / | | __| |\\  | | |    \\  | |  | | |   \\" << endl
         << "  | |   | |    |  \\_/  | | |_  | \\ | | | ||  \\ | |  | | | () |" << endl
         << "  | |   | |    |  ___  | | __| |  \\| | | ||  | | |  | | |  _ \\" << endl
         << "  | |   | |__  | |   | | | |_  |  _  | |    /  | |__| | | | \\ \\" << endl
         << "  | |   |____| |_|   |_| |___| |_| \\_| |___/   |______| |_|  \\_\\" << endl
         << "  \\ /" << endl
         << "   °               H E I R S  T O  T H E  E L V E N  S W O R D" << endl << endl;

    cout << "                                            VERSION " << ILMENDUR_VERSION << endl;

    if (ILMENDUR_VERSION_MAJOR < 1 || ILMENDUR_VERSION_MINOR % 2 == 1) {
        cout << endl << "This is a development version. Be careful." << endl << endl;
    }

    cout << "Welcome, adventurer." << endl;

    cout << "Gettext information: " << endl
         << "    Active locale: " << locname << endl
         << "    Bound translations directory: " << tbasedir << endl
         << "    Text domain: " << tdomain << endl
         << "    Domain charset: " << tcharset << endl;

    setupGlfw();
    setupOgre();

    sp_application = this;
}

Application::~Application()
{
    shutdownOgre();
    shutdownGlfw();
    sp_application = nullptr;
}

Application& Application::getSingleton()
{
    return *sp_application;
}

void Application::setupGlfw()
{
    if (!glfwInit()) {
        throw(runtime_error("Failed to initialise glfw!"));
    }

    if (!glfwJoystickPresent(GLFW_JOYSTICK_1)) {
        throw(runtime_error("No joystick found! It is required to play the game."));
    }
}

void Application::shutdownGlfw()
{
    glfwTerminate();
}

void Application::setupOgre()
{
    Ogre::Root* p_root = new Ogre::Root(""); // Note: also saved in Ogre::Root::getSingleton()
    loadOgrePlugins();
    setupOgreOverlaySystem(); // Requires Ogre::Root to not be created-but-not-initialised
    p_root->setRenderSystem(p_root->getAvailableRenderers()[0]);
    p_root->initialise(false);
}

void Application::shutdownOgre()
{
    delete Ogre::OverlaySystem::getSingletonPtr();
    delete Ogre::Root::getSingletonPtr(); // Automatically unloads plugins in correct order, see http://wiki.ogre3d.org/StaticLinking
    m_ogre_plugins.clear();
}

/* This function should load all required plugins, which in turn
 * should be all plugins listed in our Ogre build's plugins.cfg file.
 * That file is not used in static builds, but gives a nice hint on
 * what to load. */
void Application::loadOgrePlugins()
{
    m_ogre_plugins.push_back(move(make_unique<Ogre::GL3PlusPlugin>()));
    m_ogre_plugins.push_back(move(make_unique<Ogre::ParticleFXPlugin>()));
    m_ogre_plugins.push_back(move(make_unique<Ogre::STBIPlugin>()));
    m_ogre_plugins.push_back(move(make_unique<Ogre::DotScenePlugin>()));

    for(unique_ptr<Ogre::Plugin>& p_plugin: m_ogre_plugins) {
        Ogre::Root::getSingleton().installPlugin(p_plugin.get());
    }
}

void Application::loadOgreResources()
{
    fs::path ogre_internal_resource_dir = OS::ogre_resource_dir();
    fs::path ilmendur_resource_dir = OS::game_resource_dir();

    cout << "Ogre internal resources directory: " << ogre_internal_resource_dir << endl
         << "ILMENDUR resource directory: " << ilmendur_resource_dir << endl;

    /* First add Ogre's own OgreInternal resources. These are
     * taken from Ogre's resources.cfg's OgreInternal section,
     * which they have to match. */
    vector<fs::path> ogredirs = {ogre_internal_resource_dir / fs::u8path("Main"),
                                 ogre_internal_resource_dir / fs::u8path("RTShaderLib/GLSL"),
                                 ogre_internal_resource_dir / fs::u8path("RTShaderLib/HLSL_Cg"),
                                 ogre_internal_resource_dir / fs::u8path("Terrain")};
    for (const fs::path& ogredir: ogredirs) {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(ogredir.u8string(), "FileSystem", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
    }

    // Now add the project's own resources.
    // The "General" group holds global resources typically used in many scenes.
    // The groups starting with "scenes/" hold scene-specific resources.
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation((ilmendur_resource_dir / fs::u8path("general")).u8string(), "FileSystem", "General"); // Ogre convention wants the "General" group to be capitalised
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation((ilmendur_resource_dir / fs::u8path("scenes/dummy_scene")).u8string(), "FileSystem", "scenes/dummy_scene"); // Ogre convention wants the "General" group to be capitalised
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation((ilmendur_resource_dir / fs::u8path("scenes/test_scene")).u8string(), "FileSystem", "scenes/test_scene");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation((ilmendur_resource_dir / fs::u8path("meshes")).u8string(), "FileSystem", "miscmeshes");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation((ilmendur_resource_dir / fs::u8path("fonts")).u8string(), "FileSystem", "fonts");
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation((ilmendur_resource_dir / fs::u8path("ui")).u8string(), "FileSystem", "ui");

    // Initialise all the groups that have been added above
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void Application::setupOgreOverlaySystem()
{
    // Ogre::OverlaySystem constructor saves the overlay system in
    // the singleton Ogre::OverlaySystem::getSingleton(), so ignore
    // the result.
    new Ogre::OverlaySystem();
}

/**
 * Initialises Ogre's Real-Time Shading System (RTSS). Calling
 * this function requires an active OpenGL context.
 */
void Application::setupOgreRTSS()
{
    if (Ogre::RTShader::ShaderGenerator::initialize()) {
        mp_sglistener = new SGTechniqueResolverListener(Ogre::RTShader::ShaderGenerator::getSingletonPtr());
        Ogre::MaterialManager::getSingleton().addListener(mp_sglistener);
    } else {
        throw(runtime_error("Failed to initialise RTSS"));
    }
}

/**
 * Cleans up Ogre's Real-Time Shading System (RTSS). Call this
 * before destroying the OpenGL context it is used in.
 */
void Application::shutdownOgreRTSS()
{
    Ogre::MaterialManager::getSingleton().setActiveScheme(Ogre::MaterialManager::DEFAULT_SCHEME_NAME);
    Ogre::MaterialManager::getSingleton().removeListener(mp_sglistener);
    delete mp_sglistener;
    Ogre::RTShader::ShaderGenerator::destroy();
}

/// Access the Window for this application.
Window& Application::getWindow()
{
    return *mp_window;
}

SceneSystem::Scene& Application::currentScene()
{
    return *m_scene_stack.top();
}

/**
 * Pushes `p_scene' onto the scene stack. The Application instance
 * takes control of the memory of `p_scene' -- do not free it
 * yourself and do not use the pointer anymore after passing it
 * to this method.
 *
 * After this method returns, the passed Scene instance will be at
 * the top of the scene stack, thus it will be played on the next
 * iteration of the main loop. If it is popped, the currently
 * running scene will show up again on the mainloop’s next
 * iteration. If this is not desired, use popScene() to remove
 * the current scene from the scene stack before pushing your
 * new scene.
 *
 * \remark If you want to transition to a new scene and never return
 * to the scene currently at the top of the scene stack, use
 * SceneSystem::Scene::finish() with an appropriately set `p_next`
 * parameter instead, because this is easier to understand.
 */
void Application::pushScene(unique_ptr<SceneSystem::Scene> p_scene)
{
    m_scene_stack.top()->deactivate();
    m_scene_stack.push(move(p_scene));
    m_scene_stack.top()->activate();
}

/**
 * Pop the current scene from the stack. On the main loop’s
 * next iteration, whatever was below will be played. If there
 * is nothing below, the game terminates.
 *
 * If you want to store the scene before popping it, use
 * currentScene() to retrieve it before calling popScene().
 *
 * \remark This method ensures that the scene’s memory is
 * deleted. If you call this from the very same scene which
 * is popped, your `this' pointer gets deleted. Be careful
 * what you do in such cases.
 */
void Application::popScene()
{
    m_scene_stack.top()->deactivate();
    m_scene_stack.pop();
}

/**
 * Creates the window and enters the main loop.
 */
void Application::run()
{
    mp_window = new Window(1280, 720, "Ilmendur"); // 16:9 ratio
    mp_window->activate();

    // Initialising the RTSS requires an active window.
    setupOgreRTSS();
    // Loading the resources requires the RTSS to be active.
    loadOgreResources();
    // Initialising the UI system requires the Ogre resources to be loaded.
    new UISystem::GUIEngine(); // This stores the singleton pointer on itself

    // For now, only display the dummy scene
    //m_scene_stack.push(move(make_unique<SceneSystem::DummyScene>()));
    m_scene_stack.push(move(make_unique<SceneSystem::JoymenuScene>()));

    // TODO: If there is a savegame file, load it and read the joystick
    // configuration from it. Otherwise, additionally push the joystick
    // configuration scene onto the stack.

    // Register GLFW callbacks
    glfwSetKeyCallback(mp_window->getGLFWWindow(), processGLFWKeys);
    glfwSetCharCallback(mp_window->getGLFWWindow(), processGLFWChars);
    glfwSetCursorPosCallback(mp_window->getGLFWWindow(), processGLFWCursorMove);
    glfwSetMouseButtonCallback(mp_window->getGLFWWindow(), processGLFWMouseButton);

    // Main loop
    chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
    chrono::high_resolution_clock::time_point t2 = t1;
    m_scene_stack.top()->activate();
    while (m_scene_stack.size() > 0) {
        t2 = chrono::high_resolution_clock::now();
        m_fps = 1000.0f / chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();
        t1 = t2;
        //printf("FPS: %.2f\n", m_fps);

        m_scene_stack.top()->update();
        Ogre::Root::getSingleton().renderOneFrame();

        glfwSwapBuffers(mp_window->getGLFWWindow());
        glfwPollEvents();

        if (m_scene_stack.top()->isFinishing()) {
            SceneSystem::Scene* p_next_scene = m_scene_stack.top()->nextScene();
            m_scene_stack.top()->deactivate();
            m_scene_stack.pop();

            if (p_next_scene) {
                m_scene_stack.push(move(unique_ptr<SceneSystem::Scene>(p_next_scene)));
                m_scene_stack.top()->activate();
            }
        }
    }

    // Clear all remaining scenes, if any
    while (m_scene_stack.size() > 0) {
        m_scene_stack.top()->deactivate();
        m_scene_stack.pop();
    }

    // Shutdown UI system
    UISystem::GUIEngine& ui = UISystem::GUIEngine::getSingleton();
    delete &ui;

    shutdownOgreRTSS();
    delete mp_window;
    mp_window = nullptr;
}
