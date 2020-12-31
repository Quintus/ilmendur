#include <GLFW/glfw3.h>
#include <GL/gl3w.h>
#include <OGRE/Ogre.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OgreGL3PlusPlugin.h>
#include <OgreParticleFXPlugin.h>
#include <OgreSTBICodec.h>
#include "resolver.hpp"

int main(int argc, char* argv[])
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow* p_window = glfwCreateWindow(800, 600, "Hello world", nullptr, nullptr);
    if (!p_window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(p_window);

    // Create root
    Ogre::Root* p_ogre_root = new Ogre::Root("");

    // Load plugins
    Ogre::GL3PlusPlugin* p_gl3plus_plugin = new Ogre::GL3PlusPlugin();
    Ogre::ParticleFXPlugin* p_particlefx_plugin = new Ogre::ParticleFXPlugin();
    Ogre::STBIPlugin* p_stbi_plugin = new Ogre::STBIPlugin();
    p_ogre_root->installPlugin(p_gl3plus_plugin);
    p_ogre_root->installPlugin(p_particlefx_plugin);
    p_ogre_root->installPlugin(p_stbi_plugin);

    // Initialise overlay system
    Ogre::OverlaySystem* p_overlay_system = new Ogre::OverlaySystem();
    p_ogre_root->setRenderSystem(p_ogre_root->getAvailableRenderers()[0]);

    // Initialise Ogre root
    p_ogre_root->initialise(false);

    // Load resources
    Ogre::ConfigFile ogre_config;
    ogre_config.load("/home/quintus/repos/rpg/build/deps/share/OGRE/resources.cfg");
    Ogre::ConfigFile::SectionIterator seci = ogre_config.getSectionIterator();

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

    Ogre::NameValuePairList misc;
	misc["currentGLContext"] = Ogre::String("true");
	Ogre::RenderWindow* p_ogre_render_window = p_ogre_root->createRenderWindow("proofoconcept", 800, 600, false, &misc);
	p_ogre_render_window->setVisible(true);

    // Initialise render system
    if (Ogre::RTShader::ShaderGenerator::initialize()) {
        SGTechniqueResolverListener* p_sglistener = new SGTechniqueResolverListener(Ogre::RTShader::ShaderGenerator::getSingletonPtr());
        Ogre::MaterialManager::getSingleton().addListener(p_sglistener);
    }
    else {
        throw(std::runtime_error("Failed to initialise RTSS"));
    }
    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

    Ogre::RenderTarget *ogreRenderTarget = p_ogre_render_window;
    Ogre::SceneManager* p_scene_manager = p_ogre_root->createSceneManager();

    p_ogre_root->clearEventTimes();

    // Tutorial code follows
    Ogre::RTShader::ShaderGenerator* p_shadergen = Ogre::RTShader::ShaderGenerator::getSingletonPtr();
    p_shadergen->addSceneManager(p_scene_manager);

    // without light we would just get a black screen
    Ogre::Light* p_light = p_scene_manager->createLight("MainLight");
    Ogre::SceneNode* p_light_node = p_scene_manager->getRootSceneNode()->createChildSceneNode();
    p_light_node->setPosition(0, 10, 15);
    p_light_node->attachObject(p_light);

    // also need to tell where we are
    Ogre::SceneNode* p_cam_node = p_scene_manager->getRootSceneNode()->createChildSceneNode();
    p_cam_node->setPosition(0, 0, 15);
    p_cam_node->lookAt(Ogre::Vector3(0, 0, -1), Ogre::Node::TS_PARENT);

    // create the camera
    Ogre::Camera* p_camera = p_scene_manager->createCamera("myCam");
    p_camera->setNearClipDistance(5);
    p_camera->setAutoAspectRatio(true);
    p_cam_node->attachObject(p_camera);

    // Departure from tutorial. Add a viewport with the given camera
    // to the render window.
    p_ogre_render_window->addViewport(p_camera);

    // Add something into the scene
    Ogre::Entity* p_entity = p_scene_manager->createEntity("Sinbad.mesh");
    Ogre::SceneNode* p_node = p_scene_manager->getRootSceneNode()->createChildSceneNode();
    p_node->attachObject(p_entity);

    while (true) {
        //Ogre::WindowEventUtilities::messagePump();
		if(glfwGetKey(p_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			break;
		}
		p_ogre_root->renderOneFrame();
		glfwPollEvents();
    }

    // Cleanup. Plugins need to be deleted after the Ogre::Root object was deleted,
    // see http://wiki.ogre3d.org/StaticLinking.
    delete p_ogre_root;
    delete p_stbi_plugin;
    delete p_particlefx_plugin;
    delete p_gl3plus_plugin;

    glfwDestroyWindow(p_window);
    glfwTerminate();
    return 0;
}
