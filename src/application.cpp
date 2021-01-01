#include "application.hpp"
#include "resolver.hpp"
#include <string>
#include <GLFW/glfw3.h>
#include <OGRE/Ogre.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OgreGL3PlusPlugin.h>
#include <OgreParticleFXPlugin.h>
#include <OgreSTBICodec.h>

static Application* sp_application = nullptr;
static std::string s_application_title;

Application::Application()
    : _mp_scene_manager(nullptr),
      mp_sglistener(nullptr)
{
    if (sp_application)
        throw(std::runtime_error("There can only be one Application instance!"));

    // Set window title. TODO: Translate this (this is why this is
    // specified as a constant).
    s_application_title = "RPG";

    //setupGlfw();
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

/**
 * This initialises GLFW, creates a window with it and sets that window's OpenGL context
 * current. That's exactly the situation needed to call setupOgre().
 */
void Application::setupGlfw()
{
    if (!glfwInit())
        throw(std::runtime_error("Failed to initialise glfw!"));

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    // TODO: Set X11 floating hint on Linux for tiling window managers
    // GLFW has functions to retrieve the native window and display.
    // Screen on today's system is always 0 (nobody uses X anymore to
    // render for multiple computers). The Screen* pointer itself can
    // be retrieved as ScreenOfDisplay(p_x11_display, XDefaultScreen(p_x11_display)),
    // where XDefaultScreen() will usually return said 0.

    // Note: glfwCreateWindow requires the window title to be UTF-8.
    m_windowpair.glfw_window = glfwCreateWindow(800, 600, s_application_title.c_str(), nullptr, nullptr);

    if (!m_windowpair.glfw_window) {
        glfwTerminate();
        throw(std::runtime_error("Failed to create the GLFW window!"));
    }

    glfwMakeContextCurrent(m_windowpair.glfw_window);
}

void Application::shutdownGlfw()
{
    glfwDestroyWindow(m_windowpair.glfw_window);
    m_windowpair.glfw_window = nullptr;
    glfwTerminate();
}

/**
 * Initialises Ogre and creates a RenderWindow. This function requires that setupGlfw()
 * has been called before, because it relies on a current OpenGL context.
 */
void Application::setupOgre()
{
    Ogre::Root* p_root = new Ogre::Root(""); // Note: also saved in Ogre::Root::getSingleton()
    loadOgrePlugins();
    setupOgreOverlaySystem(); // Requires Ogre::Root to not be created-but-not-initialised
    p_root->setRenderSystem(p_root->getAvailableRenderers()[0]);
    p_root->initialise(false);

    setupGlfw();

    // Create Ogre's render window and configure it to just use GLFW's already
    // created OpenGL context. The height/width/fullscreen parameters are
    // thus ignored.
    Ogre::NameValuePairList misc;
    misc["currentGLContext"] = Ogre::String("true");
    misc["externalGLControl"] = Ogre::String("true");
    m_windowpair.ogre_window = p_root->createRenderWindow(s_application_title, 1, 1, false, &misc);
    m_windowpair.ogre_window->setVisible(true);

    setupOgreRTSS();
    loadOgreResources();
}

void Application::shutdownOgre()
{
    shutdownOgreRTSS();

    Ogre::Root::getSingleton().destroyRenderTarget(m_windowpair.ogre_window);
    delete Ogre::OverlaySystem::getSingletonPtr();
    delete Ogre::Root::getSingletonPtr(); // Automatically unloads plugins in correct order, see http://wiki.ogre3d.org/StaticLinking
    // But does not delete the plugins. This needs to be done manually.
    for (Ogre::Plugin* p_plugin: m_ogre_plugins)
        delete p_plugin;
    m_ogre_plugins.clear();
    m_windowpair.ogre_window = nullptr;
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

void Application::shutdownOgreRTSS()
{
    Ogre::MaterialManager::getSingleton().setActiveScheme(Ogre::MaterialManager::DEFAULT_SCHEME_NAME);
    Ogre::MaterialManager::getSingleton().removeListener(mp_sglistener);
    delete mp_sglistener;
    Ogre::RTShader::ShaderGenerator::destroy();
}

void Application::run()
{
    _make_a_scene();

    // Main loop
    while (true) {
        //Ogre::WindowEventUtilities::messagePump();
		if(glfwGetKey(m_windowpair.glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			break;
		}
        Ogre::Root::getSingleton().renderOneFrame();
        glfwSwapBuffers(m_windowpair.glfw_window);
        glfwPollEvents();
    }

    _destroy_the_scene();
}

void Application::_make_a_scene()
{
    // Make a scene manager for this scene
    _mp_scene_manager = Ogre::Root::getSingleton().createSceneManager();

    // register our scene with the RTSS
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->addSceneManager(_mp_scene_manager);

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
    m_windowpair.ogre_window->addViewport(p_camera);

    // Add something into the scene
    Ogre::Entity* p_entity = _mp_scene_manager->createEntity("Sinbad.mesh");
    Ogre::SceneNode* p_node = _mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    p_node->attachObject(p_entity);
}

void Application::_destroy_the_scene()
{
    m_windowpair.ogre_window->removeAllViewports();
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->removeSceneManager(_mp_scene_manager);
    Ogre::Root::getSingleton().destroySceneManager(_mp_scene_manager);
    _mp_scene_manager = nullptr;
}
