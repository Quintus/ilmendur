#include "dummy_scene.hpp"
#include "../core/application.hpp"
#include "../core/window.hpp"
#include <GLFW/glfw3.h>
#include <OGRE/Ogre.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>

using namespace SceneSystem;

DummyScene::DummyScene()
    : Scene("dummy scene"),
      mp_cube_node(nullptr)
{
    // register our scene with the RTSS
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->addSceneManager(mp_scene_manager);

    // Enable the overlay system for this scene
    mp_scene_manager->addRenderQueueListener(Ogre::OverlaySystem::getSingletonPtr());

    // without light we would just get a black screen
    Ogre::Light* p_light = mp_scene_manager->createLight("MainLight");
    Ogre::SceneNode* p_light_node = mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    p_light_node->setPosition(0, 10, 15);
    p_light_node->attachObject(p_light);

    // also need to tell where we are
    Ogre::SceneNode* p_cam_node = mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    p_cam_node->setPosition(0, 0, 15);
    p_cam_node->lookAt(Ogre::Vector3(0, 0, -1), Ogre::Node::TS_PARENT);

    // create the camera
    Ogre::Camera* p_camera = mp_scene_manager->createCamera("myCam");
    p_camera->setNearClipDistance(5);
    p_camera->setAutoAspectRatio(true);
    p_cam_node->attachObject(p_camera);

    // Departure from tutorial. Add a viewport with the given camera
    // to the render window.
    Core::Application::getSingleton().getWindow().getOgreRenderWindow()->addViewport(p_camera);

    // Add something into the scene
    Ogre::Entity* p_entity = mp_scene_manager->createEntity("Cube.mesh");
    mp_cube_node = mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    mp_cube_node->attachObject(p_entity);
}

DummyScene::~DummyScene()
{
    Core::Application::getSingleton().getWindow().getOgreRenderWindow()->removeAllViewports();
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->removeSceneManager(mp_scene_manager);
}

void DummyScene::update()
{
    if(glfwGetKey(Core::Application::getSingleton().getWindow().getGLFWWindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        finish();
    }

    mp_cube_node->rotate(Ogre::Vector3(0, 0, 1), Ogre::Degree(1));
    mp_cube_node->rotate(Ogre::Vector3(0, 1, 0), Ogre::Degree(1));
}
