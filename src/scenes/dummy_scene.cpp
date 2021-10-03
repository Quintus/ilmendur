#include "dummy_scene.hpp"
#include "../core/application.hpp"
#include "../core/window.hpp"
#include "../audio/music.hpp"
#include "../actors/static_geometry.hpp"
#include "../actors/freya.hpp"
#include <GLFW/glfw3.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OGRE/OgreMath.h>
#include <btBulletDynamicsCommon.h>
#include <iostream>

using namespace std;
using namespace SceneSystem;

DummyScene::DummyScene()
    : Scene("dummy scene"),
      mp_area_node(nullptr),
      mp_cam_node(nullptr),
      mp_ground(nullptr),
      mp_player(nullptr)
{
    // Enable physics
    mp_physics = new PhysicsSystem::PhysicsEngine(mp_scene_manager->getRootSceneNode());

    // register our scene with the RTSS
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->addSceneManager(mp_scene_manager);

    // Enable the overlay system for this scene
    mp_scene_manager->addRenderQueueListener(Ogre::OverlaySystem::getSingletonPtr());

    // Sky box
    mp_scene_manager->setSkyBox(true, "testskybox");

    // without light we would just get a black screen
    mp_scene_manager->setAmbientLight(Ogre::ColourValue(0.5, 0.5, 0.5));

    // also need to tell where we are
    mp_cam_node = mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    mp_cam_node->setPosition(20, 20, 1.75);
    /* Align camera with Blender's axes. Blender has Z pointing upwards
     * while Ogre's camera by default faces down the Z axis, i.e. Ogre
     * treats the Y axis as the height. The below line rotates the camera
     * so that it look in -Y axis direction, making the +Z axis the upwards direction
     * like it is in Blender. */
    mp_cam_node->setOrientation(Ogre::Quaternion(Ogre::Degree(180), Ogre::Vector3::UNIT_Z) * Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_X));

    // create the camera
    Ogre::Camera* p_camera = mp_scene_manager->createCamera("myCam");
    p_camera->setNearClipDistance(0.1);
    p_camera->setAutoAspectRatio(true);
    mp_cam_node->attachObject(p_camera);

    // Add a viewport with the given camera to the render window.
    Core::Application::getSingleton().getWindow().getOgreRenderWindow()->addViewport(p_camera);

    // Load the test area. Note the test area model has the floor in the XY plane.
    mp_area_node = mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    Ogre::ResourceGroupManager::getSingleton().setWorldResourceGroupName("scenes/test_scene");
    mp_area_node->loadChildren("testscene.scene");
    replaceBlenderEntities(mp_area_node);

    // Extract ground from the loaded scene and add it into the physics system
    mp_ground = new StaticGeometry(*this, mp_scene_manager->getSceneNode("Ground"));
    mp_physics->addActor(mp_ground);

    // Add player figure
    mp_player = new Freya(*this);
    mp_player->setPosition(Ogre::Vector3(25, 0, 10));
    mp_physics->addActor(mp_player);
}

DummyScene::~DummyScene()
{
    mp_physics->removeActor(mp_player);
    mp_physics->removeActor(mp_ground);

    delete mp_player;
    delete mp_ground;
    delete mp_physics;

    Core::Application::getSingleton().getWindow().getOgreRenderWindow()->removeAllViewports();
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->removeSceneManager(mp_scene_manager);
}

void DummyScene::update()
{
    mp_physics->update();
}

void DummyScene::processKeyInput(int key, int scancode, int action, int mods)
{

    Ogre::Vector3 dir = mp_cam_node->getOrientation().zAxis() * -1;
    dir.normalise();

    switch (key) {
    case GLFW_KEY_ESCAPE:
        finish();
        break;
    case GLFW_KEY_UP:
        mp_cam_node->translate(dir * 0.5);
        break;
    case GLFW_KEY_DOWN:
        mp_cam_node->translate(-dir * 0.5);
        break;
    case GLFW_KEY_LEFT:
        mp_cam_node->yaw(Ogre::Angle(1));
        break;
    case GLFW_KEY_RIGHT:
        mp_cam_node->yaw(Ogre::Angle(-1));
        break;
    case GLFW_KEY_PAGE_UP:
        mp_cam_node->pitch(Ogre::Angle(1));
        break;
    case GLFW_KEY_PAGE_DOWN:
        mp_cam_node->pitch(Ogre::Angle(-1));
        break;
    case GLFW_KEY_X:
        cout << "Repositioning!" << endl;
        mp_player->reposition(Ogre::Vector3(25, 0, 10), Ogre::Quaternion(Ogre::Degree(0), Ogre::Vector3::UNIT_Y));
        break;
        //default:
        // Ignore
    }
}
