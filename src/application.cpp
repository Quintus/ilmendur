#include "application.hpp"
#include "resolver.hpp"
#include "window.hpp"
#include <GLFW/glfw3.h>
#include <OGRE/Ogre.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OgreGL3PlusPlugin.h>
#include <OgreParticleFXPlugin.h>
#include <OgreSTBICodec.h>

static Application* sp_application = nullptr;

Application::Application()
    : _mp_scene_manager(nullptr),
      mp_window(nullptr),
      mp_sglistener(nullptr)
{
    if (sp_application)
        throw(std::runtime_error("There can only be one Application instance!"));

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
    if (!glfwInit())
        throw(std::runtime_error("Failed to initialise glfw!"));
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
    for (Ogre::Plugin* p_plugin: m_ogre_plugins)
        delete p_plugin;
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

    for(Ogre::Plugin* p_plugin: m_ogre_plugins)
        Ogre::Root::getSingleton().installPlugin(p_plugin);
}

void Application::loadOgreResources()
{
    // TODO: For now, this loads Ogre's example resources. Of course,
    // we want to replace this with our own ones!
    Ogre::ConfigFile resconfigfile;
    resconfigfile.load("deps/share/OGRE/resources.cfg"); // Assumes build dir as current working directory

    // TODO: Change this to look more like OgreBites' ApplicationContextBase::locateResources()
    Ogre::ConfigFile::SectionIterator seci = resconfigfile.getSectionIterator();
	Ogre::String secName, typeName, archName;
	while (seci.hasMoreElements()) {
		secName = seci.peekNextKey();
		Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
		Ogre::ConfigFile::SettingsMultiMap::iterator i;
		for (i = settings->begin(); i != settings->end(); ++i) {
			typeName = i->first;
			archName = i->second;
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
			archName, typeName, secName);
		}
	}

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
    }
    else {
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
    Ogre::Entity* p_entity = _mp_scene_manager->createEntity("Sinbad.mesh");
    Ogre::SceneNode* p_node = _mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    p_node->attachObject(p_entity);
}

void Application::_destroy_the_scene()
{
    mp_window->getOgreRenderWindow()->removeAllViewports();
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->removeSceneManager(_mp_scene_manager);
    Ogre::Root::getSingleton().destroySceneManager(_mp_scene_manager);
    _mp_scene_manager = nullptr;
}
