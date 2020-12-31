#include "application.hpp"
#include "resolver.hpp"
#include <GLFW/glfw3.h>
#include <GL/gl3w.h>  // Ogre's OpenGL header
#include <OGRE/Ogre.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OgreGL3PlusPlugin.h>
#include <OgreParticleFXPlugin.h>
#include <OgreSTBICodec.h>


static Application* sp_application = nullptr;
//static void* sp_ignore_because_singleton = nullptr;

Application::Application()
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    mp_glfw_window = glfwCreateWindow(800, 600, "RPG", nullptr, nullptr);
    if (!mp_glfw_window) {
        glfwTerminate();
        throw(std::runtime_error("Failed to create a GLFW window!"));
    }
}

void Application::shutdownGlfw()
{
    glfwDestroyWindow(mp_glfw_window);
    glfwTerminate();
}

void Application::setupOgre()
{
    // Switch to that window's OpenGL context. Ogre must have the context
    // active during setup.
    glfwMakeContextCurrent(mp_glfw_window);

    Ogre::Root* p_root = new Ogre::Root(""); // Note: also saved in Ogre::Root::getSingleton()
    loadOgrePlugins();
    setupOgreOverlaySystem();
    p_root->setRenderSystem(p_root->getAvailableRenderers()[0]);
    p_root->initialise(false);
    setupOgreRenderWindow();
    loadOgreResources();
    p_root->clearEventTimes();
}

void Application::shutdownOgre()
{
    shutdownOgreRenderWindow();
    delete Ogre::OverlaySystem::getSingletonPtr();
    unloadOgrePlugins();
    delete Ogre::Root::getSingletonPtr();
}

/* This function should load all required plugins, which in turn
 * should be all plugins listed in our Ogre build's plugins.cfg file.
 * That file is not used in static builds, but gives a nice hint on
 * what to load. */
void Application::loadOgrePlugins()
{
    // Plugins, on which other plugins depend, should come first.
    // That's why the renderer plugin should be the first one.
    // unloadOgrePlugins() unloads plugins in reverse order.
    m_ogre_plugins.push_back(new Ogre::GL3PlusPlugin());
    m_ogre_plugins.push_back(new Ogre::ParticleFXPlugin());
    m_ogre_plugins.push_back(new Ogre::STBIPlugin());

    for(Ogre::Plugin* p_plugin: m_ogre_plugins)
        Ogre::Root::getSingleton().installPlugin(p_plugin);
}

void Application::unloadOgrePlugins()
{
    // Unload plugins in reverse order to ensure dependencies are
    // honoured (cf. loadOgrePlugins()).
    for(auto iter = m_ogre_plugins.rbegin(); iter != m_ogre_plugins.rend(); iter++) {
        Ogre::Root::getSingleton().uninstallPlugin(*iter);
        delete (*iter);
    }
    m_ogre_plugins.clear();
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

void Application::setupOgreRenderWindow()
{
    /* Create a render window that matches the GLFW window. The important thing
     * is the currentGLContext option, as documented for Ogre::Root::createRenderWindow().
     * It tells Ogre to just accept out already set up OpenGL context (which was
     * set up with GLFW in setupGlfw(). */
    static Ogre::NameValuePairList misc_params;
	misc_params["currentGLContext"] = Ogre::String("true");
	Ogre::RenderWindow* p_ogre_render_window = Ogre::Root::getSingleton().createRenderWindow("RPGRenderWindow", 800, 600, false, &misc_params);
	p_ogre_render_window->setVisible(true);

    // Initialise Real Time Shader System (RTSS)
    if (Ogre::RTShader::ShaderGenerator::initialize()) {
        mp_sglistener = new SGTechniqueResolverListener(Ogre::RTShader::ShaderGenerator::getSingletonPtr());
        Ogre::MaterialManager::getSingleton().addListener(mp_sglistener);
    }
    else {
        throw(std::runtime_error("Failed to initialise RTSS"));
    }
}

void Application::shutdownOgreRenderWindow()
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
		if(glfwGetKey(mp_glfw_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			break;
		}
        Ogre::Root::getSingleton().renderOneFrame();
		glfwPollEvents();
    }

    _destroy_the_scene();
}

void Application::_make_a_scene()
{
    // Make a scene manager for this scene
    Ogre::SceneManager* _mp_scene_manager = Ogre::Root::getSingleton().createSceneManager();

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
    Ogre::Root::getSingleton().getRenderTarget("RPGRenderWindow")->addViewport(p_camera);

    // Add something into the scene
    Ogre::Entity* p_entity = _mp_scene_manager->createEntity("Sinbad.mesh");
    Ogre::SceneNode* p_node = _mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    p_node->attachObject(p_entity);
}

void Application::_destroy_the_scene()
{
    Ogre::Root::getSingleton().getRenderTarget("RPGRenderWindow")->removeAllViewports();
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->removeSceneManager(_mp_scene_manager);
    Ogre::Root::getSingleton().destroySceneManager(_mp_scene_manager);
}
