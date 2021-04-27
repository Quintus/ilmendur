#include "application.hpp"
#include "resolver.hpp"
#include "window.hpp"
#include "../os/paths.hpp"
#include "../scenes/dummy_scene.hpp"
#include <buildconfig.hpp>
#include <GLFW/glfw3.h>
#include <OGRE/Ogre.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OgreGL3PlusPlugin.h>
#include <OgreParticleFXPlugin.h>
#include <OgreSTBICodec.h>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace std;
using namespace Core;

static Application* sp_application = nullptr;

Application::Application()
    : mp_window(nullptr),
      mp_sglistener(nullptr)
{
    if (sp_application) {
        throw(runtime_error("There can only be one Application instance!"));
    }

    cout << "Welcome, adventurer, to project version " << RPG_VERSION << "." << endl;
    if (RPG_VERSION_MAJOR < 1 || RPG_VERSION_MINOR % 2 == 1) {
        cout << "This is a development version. Be careful." << endl;
    }

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

    for(unique_ptr<Ogre::Plugin>& p_plugin: m_ogre_plugins) {
        Ogre::Root::getSingleton().installPlugin(p_plugin.get());
    }
}

void Application::loadOgreResources()
{
    fs::path ogre_internal_resource_dir = OS::ogre_resource_dir();
    fs::path rpg_resource_dir = OS::game_resource_dir();

    cout << "Ogre internal resources directory: " << ogre_internal_resource_dir << endl
         << "RPG resource directory: " << rpg_resource_dir << endl;

    /* First add Ogre's own OgreInternal resources. These are
     * taken from Ogre's resources.cfg's OgreInternal section,
     * which they have to match. */
    vector<fs::path> ogredirs = {ogre_internal_resource_dir / fs::u8path("ShadowVolume"),
                                 ogre_internal_resource_dir / fs::u8path("RTShaderLib/materials"),
                                 ogre_internal_resource_dir / fs::u8path("RTShaderLib/GLSL"),
                                 ogre_internal_resource_dir / fs::u8path("RTShaderLib/HLSL_Cg"),
                                 ogre_internal_resource_dir / fs::u8path("Terrain")};
    for (const fs::path& ogredir: ogredirs) {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(ogredir.u8string(), "FileSystem", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
    }

    // Now add the project's own resources
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation((rpg_resource_dir / fs::u8path("meshes")).u8string(), "FileSystem", "General");

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

/**
 * Creates the window and enters the main loop.
 */
void Application::run()
{
    mp_window = new Window(800, 600, "RPG");
    mp_window->activate();

    // Initialising the RTSS requires an active window.
    setupOgreRTSS();
    // Loading the resources requires the RTSS to be active.
    loadOgreResources();

    // For now, only display the dummy scene
    m_scene_stack.push(move(make_unique<SceneSystem::DummyScene>()));

    // Main loop
    while (m_scene_stack.size() > 0) {
        m_scene_stack.top()->update();

        Ogre::Root::getSingleton().renderOneFrame();
        glfwSwapBuffers(mp_window->getGLFWWindow());
        glfwPollEvents();

        if (m_scene_stack.top()->isFinishing()) {
            m_scene_stack.pop();
        }
    }

    // Clear all remaining scenes, if any
    while (m_scene_stack.size() > 0) {
        m_scene_stack.pop();
    }

    shutdownOgreRTSS();
    delete mp_window;
    mp_window = nullptr;
}
