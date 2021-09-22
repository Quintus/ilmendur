#include "dummy_scene.hpp"
#include "../core/application.hpp"
#include "../core/window.hpp"
#include "../audio/music.hpp"
#include <chrono>
#include <iostream>
#include <GLFW/glfw3.h>
#include <OGRE/Ogre.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OGRE/OgreMath.h>
#include <btBulletDynamicsCommon.h>

using namespace std;
using namespace SceneSystem;

chrono::time_point<chrono::high_resolution_clock> last_physics_update;

DummyScene::DummyScene()
    : Scene("dummy scene"),
      mp_area_node(nullptr)
{
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
    mp_cam_node->setPosition(0, 0, 1.75);
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

    // Add player figure
    mp_player_node = mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    mp_player_node->setPosition(Ogre::Vector3(25, 0, 2));
    mp_player_node->attachObject(mp_scene_manager->createEntity("freya.mesh"));

    // Initialise physics
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);
    btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;
    mp_bullet_world = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);
    mp_bullet_world->setGravity(btVector3(0, 0, -10));

    Ogre::Vector3 pos = mp_player_node->getPosition();
    Ogre::Vector3 hs = mp_player_node->getAttachedObject(0)->getBoundingBox().getHalfSize();
    btScalar mass(0.63); // kg/100

    btCollisionShape* player_shape = new btBoxShape(btVector3(btScalar(hs.x), btScalar(hs.y), btScalar(hs.z)));
    btTransform player_transform;
    player_transform.setIdentity();
    player_transform.setOrigin(btVector3(pos.x, pos.y, pos.z));
    btVector3 local_inertia(0, 0, 0);
    player_shape->calculateLocalInertia(mass, local_inertia);

    btDefaultMotionState* myMotionState = new btDefaultMotionState(player_transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, player_shape, local_inertia);
    btRigidBody* player_rbody = new btRigidBody(rbInfo);
    mp_bullet_world->addRigidBody(player_rbody);

    last_physics_update = chrono::high_resolution_clock::now();
}

DummyScene::~DummyScene()
{
    Core::Application::getSingleton().getWindow().getOgreRenderWindow()->removeAllViewports();
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->removeSceneManager(mp_scene_manager);
}

void DummyScene::update()
{
    chrono::time_point<chrono::high_resolution_clock> now = chrono::high_resolution_clock::now();
    mp_bullet_world->stepSimulation(chrono::duration_cast<chrono::microseconds>(now - last_physics_update).count() / 1000000.0);
    last_physics_update = now;

    cout << "The bullet world has " << mp_bullet_world->getNumCollisionObjects() << " objects." << endl;
    for(int i=0; i < mp_bullet_world->getNumCollisionObjects(); i++) {
        btRigidBody* p_body = btRigidBody::upcast(mp_bullet_world->getCollisionObjectArray()[i]);
        if (p_body) {
            btTransform transform;
            p_body->getMotionState()->getWorldTransform(transform);
            cout << "Player is now at X=" << transform.getOrigin().getX() << " Y=" << transform.getOrigin().getY() << " Z=" << transform.getOrigin().getZ() << endl;
        }
        else {
            cout << "Encountered something else than a rigid body..." << endl;
        }
    }
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
        mp_area_node->translate(Ogre::Vector3(0, 0, 1));
        break;
        //default:
        // Ignore
    }
}
