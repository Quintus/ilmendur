#include "application.hpp"
#include "resolver.hpp"
#include "window.hpp"
#include "buildconfig.hpp"
#include <GLFW/glfw3.h>
#include <OGRE/Ogre.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OgreGL3PlusPlugin.h>
#include <OgreParticleFXPlugin.h>
#include <OgreSTBICodec.h>
#include <iostream>
#include <filesystem>

// For exe_path()
#ifdef __linux
#include <unistd.h>
#include <limits.h>
#endif

namespace fs = std::filesystem;

static Application* sp_application = nullptr;

/// Returns the path to the currently running executable.
static fs::path exe_path()
{
#if defined(__linux__)
    char buf[PATH_MAX];
    ssize_t size = readlink("/proc/self/exe", buf, PATH_MAX);
    if (size < 0)
        throw(std::runtime_error("Failed to read /proc/self/exe"));
    return fs::path(std::string(buf, size));
#else
#error Dont know how to determine path to running executable on this platform
#endif
}

Application::Application()
    : _mp_scene_manager(nullptr),
      mp_window(nullptr),
      mp_sglistener(nullptr)
{
    if (sp_application) {
        throw(std::runtime_error("There can only be one Application instance!"));
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

Application* Application::getSingleton()
{
    return sp_application;
}

void Application::setupGlfw()
{
    if (!glfwInit()) {
        throw(std::runtime_error("Failed to initialise glfw!"));
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
    // But does not delete the plugins. This needs to be done manually.
    for (Ogre::Plugin* p_plugin: m_ogre_plugins) {
        delete p_plugin;
    }
    m_ogre_plugins.clear();
}

/* This function should load all required plugins, which in turn
 * should be all plugins listed in our Ogre build's plugins.cfg file.
 * That file is not used in static builds, but gives a nice hint on
 * what to load. */
void Application::loadOgrePlugins()
{
    m_ogre_plugins.push_back(new Ogre::GL3PlusPlugin());
    m_ogre_plugins.push_back(new Ogre::ParticleFXPlugin());
    m_ogre_plugins.push_back(new Ogre::STBIPlugin());

    for(Ogre::Plugin* p_plugin: m_ogre_plugins) {
        Ogre::Root::getSingleton().installPlugin(p_plugin);
    }
}

void Application::loadOgreResources()
{
    fs::path ogre_internal_resource_dir;
    fs::path rpg_resource_dir;

#ifdef RPG_DEBUG_BUILD
    if (fs::exists(exe_path().parent_path() / fs::u8path(u8"CMakeCache.txt"))) {
        std::cout << "[NOTE] Detected running from build directory (CMakeCache.txt present). Resources will be loaded from the build and source directories, not from the installation directory." << std::endl;

        ogre_internal_resource_dir = exe_path().parent_path() / fs::u8path(u8"deps-source/ogre/Media"); // = ${CMAKE_BINARY_DIR}/deps-source/ogre/Media
        rpg_resource_dir           = fs::u8path(RPG_SOURCE_DIR) / fs::u8path(u8"data/meshes");
    } else {
#endif
        ogre_internal_resource_dir = fs::u8path(RPG_DATADIR) / fs::u8path(u8"ogre");
        rpg_resource_dir           = fs::u8path(RPG_DATADIR) / fs::u8path(u8"meshes");
#ifdef RPG_DEBUG_BUILD
    }
#endif

    std::cout << "Ogre internal resources directory: " << ogre_internal_resource_dir << std::endl
              << "RPG resource directory: " << rpg_resource_dir << std::endl;

    /* First add Ogre's own OgreInternal resources. These are
     * taken from Ogre's resources.cfg's OgreInternal section,
     * which they have to match. */
    std::vector<fs::path> ogredirs = {ogre_internal_resource_dir / fs::u8path(u8"ShadowVolume"),
                                      ogre_internal_resource_dir / fs::u8path(u8"RTShaderLib/materials"),
                                      ogre_internal_resource_dir / fs::u8path(u8"RTShaderLib/GLSL"),
                                      ogre_internal_resource_dir / fs::u8path(u8"RTShaderLib/HLSL_Cg"),
                                      ogre_internal_resource_dir / fs::u8path(u8"Terrain")};
    for (const fs::path& ogredir: ogredirs) {
        Ogre::ResourceGroupManager::getSingleton().addResourceLocation(ogredir.u8string(), "FileSystem", Ogre::ResourceGroupManager::INTERNAL_RESOURCE_GROUP_NAME);
    }

    // Now add the project's own resources
    Ogre::ResourceGroupManager::getSingleton().addResourceLocation(rpg_resource_dir, "FileSystem", "General");

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
        throw(std::runtime_error("Failed to initialise RTSS"));
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

    _make_a_scene();

    // Main loop
    while (true) {
        if(glfwGetKey(mp_window->getGLFWWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }
        Ogre::Root::getSingleton().renderOneFrame();
        glfwSwapBuffers(mp_window->getGLFWWindow());
        glfwPollEvents();
    }

    _destroy_the_scene();
    shutdownOgreRTSS();
    delete mp_window;
    mp_window = nullptr;
}

void Application::_make_a_scene()
{
    // Make a scene manager for this scene
    _mp_scene_manager = Ogre::Root::getSingleton().createSceneManager();

    // register our scene with the RTSS
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->addSceneManager(_mp_scene_manager);

    // Enable the overlay system for this scene
    _mp_scene_manager->addRenderQueueListener(Ogre::OverlaySystem::getSingletonPtr());

    // without light we would just get a black screen
    Ogre::Light* p_light = _mp_scene_manager->createLight("MainLight");
    Ogre::SceneNode* p_light_node = _mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    p_light_node->setPosition(0, 10, 15);
    p_light_node->attachObject(p_light);

    // also need to tell where we are
    Ogre::SceneNode* p_cam_node = _mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    p_cam_node->setPosition(0, 0, 15);
    p_cam_node->lookAt(Ogre::Vector3(0, 0, -1), Ogre::Node::TS_PARENT);

    // create the camera
    Ogre::Camera* p_camera = _mp_scene_manager->createCamera("myCam");
    p_camera->setNearClipDistance(5);
    p_camera->setAutoAspectRatio(true);
    p_cam_node->attachObject(p_camera);

    // Departure from tutorial. Add a viewport with the given camera
    // to the render window.
    mp_window->getOgreRenderWindow()->addViewport(p_camera);

    // Add something into the scene
    Ogre::Entity* p_entity = _mp_scene_manager->createEntity("Cube.mesh");
    Ogre::SceneNode* p_node = _mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    p_node->rotate(Ogre::Vector3(0, 0, 1), Ogre::Degree(30));
    p_node->rotate(Ogre::Vector3(0, 1, 0), Ogre::Degree(35));
    p_node->attachObject(p_entity);
}

void Application::_destroy_the_scene()
{
    mp_window->getOgreRenderWindow()->removeAllViewports();
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->removeSceneManager(_mp_scene_manager);
    Ogre::Root::getSingleton().destroySceneManager(_mp_scene_manager);
    _mp_scene_manager = nullptr;
}
