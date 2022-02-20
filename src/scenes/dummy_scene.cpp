#include "dummy_scene.hpp"
#include "../core/application.hpp"
#include "../core/camera_functions.hpp"
#include "../core/window.hpp"
#include "../core/game_state.hpp"
#include "../core/timer.hpp"
#include "../audio/music.hpp"
#include "../actors/static_geometry.hpp"
#include "../actors/freya.hpp"
#include <GLFW/glfw3.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OGRE/OgreMath.h>
#include <btBulletDynamicsCommon.h>
#include <iostream>
#include <cmath>

using namespace std;
using namespace SceneSystem;
using namespace Core;

const float CAMERA_HEIGHT = 1.0f;

// The minimum distance the camera has from the player.
const float CAMERA_MINDIST = 5.0f;

DummyScene::DummyScene()
    : Scene("dummy scene"),
      mp_area_node(nullptr),
      mp_camera_target(nullptr),
      mp_cam_node(nullptr),
      mp_ground(nullptr),
      mp_player(nullptr),
      m_run_threshold(0.0f)
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
    mp_physics->lockRotation(mp_player);

    // This special object is always inside the player by means of update(),
    // but without the player's rotation. This allows to attach the camera
    // to it and control the camera independently from the player's current
    // rotation.
    mp_camera_target = mp_scene_manager->getRootSceneNode()->createChildSceneNode();

    // also need to tell where we are
    mp_cam_node = mp_camera_target->createChildSceneNode();
    mp_cam_node->setPosition(-CAMERA_MINDIST, 0, 0);
    /* Align camera with Blender's axes. Blender has Z pointing upwards
     * while Ogre's camera by default faces down the Z axis, i.e. Ogre
     * treats the Y axis as the height. The below line rotates the camera
     * so that it look in -Y axis direction, making the +Z axis the upwards direction
     * like it is in Blender. */
    //mp_cam_node->setOrientation(Ogre::Quaternion(Ogre::Degree(180), Ogre::Vector3::UNIT_Z) * Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_X));
    mp_cam_node->setOrientation(Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_X) * Ogre::Quaternion(Ogre::Degree(270), Ogre::Vector3::UNIT_Y));

    // create the camera
    Ogre::Camera* p_camera = mp_scene_manager->createCamera("myCam");
    p_camera->setNearClipDistance(0.1);
    p_camera->setAutoAspectRatio(true);
    mp_cam_node->attachObject(p_camera);

    // Add a viewport with the given camera to the render window.
    Core::Application::getSingleton().getWindow().getOgreRenderWindow()->addViewport(p_camera);

    calculateJoyZones();
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
    //handleJoyInput();
    ////adjustCamera();

    mp_camera_target->setPosition(mp_player->getSceneNode()->getPosition());
    handleJoyInput();

    //static Timer t(10.0f, true, [this](){handleJoyInput();});
    //t.update();
}

void DummyScene::handleJoyInput()
{
    handleCamJoyInput();
    handleMoveJoyInput();
}

void DummyScene::handleCamJoyInput()
{
    const auto& config = GameState::instance.config[FREYA];
    const float* joyaxes = nullptr;
    int axescount = 0;
    joyaxes = glfwGetJoystickAxes(config.joy_index, &axescount);
    if (!joyaxes) {
        throw("Joystick removed during play");
    }

    // Any values in the dead zone are to be treated as zero.
    // It would be confusing if the dead value is used just because
    // the other axis is outside of the dead zone.
    Ogre::Vector3 vec(joyaxes[config.joy_cam_horizontal.axisno], joyaxes[config.joy_cam_vertical.axisno], 0.0f);
    if (fabs(vec.x) < config.joy_dead_zone) {
        vec.x = 0.0f;
    }
    if (fabs(vec.y) < config.joy_dead_zone) {
        vec.y = 0.0f;
    }
    if (vec.isZeroLength()) {
        return;
    }

    // Normalise out inverted axes so that UP and LEFT are always the
    // negative values and DOWN and RIGHT always the positive ones.
    if (config.joy_cam_vertical.inverted) {
        vec.y *= -1;
    }
    if (config.joy_cam_horizontal.inverted) {
        vec.x *= -1;
    }

    /* Camera rotation around the player around the Z axis within the X/Y plane.
     * The mp_cam_node is a child of mp_camera_target and placed CAMERA_MINDIST
     * units off it, so rotating the mp_camera_target makes the camera fly around
     * the player. */
    mp_camera_target->rotate(Ogre::Vector3::UNIT_Z, Ogre::Degree(vec.x * 2.5f));

    /* Camera distance (“zooming”) from the player within the X/Z plane, i.e.
     * the following leaves the Y axis alone. Note that the
     * neutral position for mp_cam_node is (-CAMERA_MINDIST, 0, 0).
     * The camera in/out movement is relative to the current position, i.e.
     * the camera is only moved if the joystick on its vertical axis (Y) is
     * outside of the dead zone and is continued to move until it either returns
     * into the dead zone or the camera limit is hit (the latter is determined
     * by the camera function in use). The camera functions are expected to return
     * a value between 0 (no movement) and 1 (very fast movement). They receive
     * the current position on the camera Z function as input and return the new
     * position onthe camera Z function, plus, to faciliate the API, the new X
     * value as well (this allows to move the camera zoom velocity code into
     * the camera function as it may not be equal for all camera functions).
     * In order to have the camera Z function receive values between 0 and 1 as
     * input, it is required to remove the -CAMERA_MINDIST offset which is set
     * on mp_cam_node before passing the current X position into the camera Z
     * function. After the camera Z function returns, the CAMERA_MINDIST needs
     * to be applied again. Both is done below. */
    if (fabs(vec.y) > config.joy_dead_zone) {
        float x = mp_cam_node->getPosition().x + CAMERA_MINDIST;
        float z = mp_cam_node->getPosition().z;
        CameraFunctions::linearCamera(vec.y, x, z);
        mp_cam_node->setPosition(Ogre::Vector3(x - CAMERA_MINDIST, 0, z));
        mp_cam_node->lookAt(mp_camera_target->getPosition(), Ogre::Node::TS_WORLD);
    }
}

void DummyScene::handleMoveJoyInput()
{
    const auto& config = GameState::instance.config[FREYA];
    const float* joyaxes = nullptr;
    int axescount = 0;
    joyaxes = glfwGetJoystickAxes(config.joy_index, &axescount);
    if (!joyaxes) {
        throw("Joystick removed during play");
    }

    // Any values in the dead zone are to be treated as zero.
    // It would be confusing if the dead value is used just because
    // the other axis is outside of the dead zone.
    Ogre::Vector2 vec(joyaxes[config.joy_horizontal.axisno], joyaxes[config.joy_vertical.axisno]);
    if (fabs(vec.x) < config.joy_dead_zone) {
        vec.x = 0.0f;
    }
    if (fabs(vec.y) < config.joy_dead_zone) {
        vec.y = 0.0f;
    }
    if (vec.isZeroLength()) {
        // Immediately stop moving when the player leaves the joystick alone
        mp_physics->setVelocity(mp_player, Ogre::Vector2(0.0f, 0.0f));
        return;
    }

    // Normalise out inverted axes so that UP and LEFT are always the
    // negative values and DOWN and RIGHT always the positive ones.
    if (config.joy_vertical.inverted) {
        vec.y *= -1;
    }
    if (config.joy_horizontal.inverted) {
        vec.x *= -1;
    }

    /* The goal is to rotate the player figure relative to the camera
     * direction by the offset that `vec' has from the joystick's Y
     * axis. For that, first calculate the angle offset from the
     * joystick's Y axis. Then, align the player figure with the
     * camera. Then, rotate the calculated amount of offset around the
     * Z axis. */
    Ogre::Radian angle_rad(vec.angleTo(-Ogre::Vector2::UNIT_Y));

    // Align the player with the camera looking direction. Models are
    // required to face down UNIT_X as per the instructions for
    // artists, and the Ogre camera also faces down X by default.
    Ogre::Quaternion cam_orient    = mp_camera_target->getOrientation();
    Ogre::Quaternion player_orient = mp_player->getSceneNode()->getOrientation();
    Ogre::Vector3 cam_lookdir      = cam_orient * Ogre::Vector3::UNIT_X;
    Ogre::Vector3 player_lookdir   = player_orient * Ogre::Vector3::UNIT_X;
    cam_lookdir.normalise();
    player_lookdir.normalise();

    Ogre::Quaternion deltaq         = player_lookdir.getRotationTo(cam_lookdir, Ogre::Vector3::UNIT_Z);
    Ogre::Quaternion joystickoffset = Ogre::Quaternion(angle_rad, Ogre::Vector3::UNIT_Z);
    mp_player->getSceneNode()->rotate(deltaq * joystickoffset);

    // Get the new orientation
    player_lookdir = mp_player->getSceneNode()->getOrientation() * Ogre::Vector3::UNIT_X;
    player_lookdir.normalise();

    // Depending on how strong is pressed, move fast or slow forward
    // into this direction.
    if (vec.length() > m_run_threshold) {
        player_lookdir *= 8;
    }
    else {
        player_lookdir *= 2;
    }

    mp_physics->setVelocity(mp_player, Ogre::Vector2(player_lookdir.x, player_lookdir.y));
    mp_physics->resetActor(mp_player, false);
}

void DummyScene::adjustCamera()
{
    Ogre::Vector3 player_pos = mp_player->getSceneNode()->getPosition();
    Ogre::Quaternion player_orient = mp_player->getSceneNode()->getOrientation();

    Ogre::Vector3 lookdir = player_orient * Ogre::Vector3::UNIT_X; // Reads looking direction
    lookdir.normalise();
    lookdir *= 3;

    printf("Lookdir: X=%.2f Y=%.2f Z=%.2f\n", lookdir.x, lookdir.y, lookdir.z);

    Ogre::Vector3 newpos = player_pos - lookdir + Ogre::Vector3(0.0f, 0.0f, CAMERA_HEIGHT);

    printf("Playerpos: X=%.2f Y=%.2f Z=%.2f\n", player_pos.x, player_pos.y, player_pos.z);
    printf("Campos: X=%.2f Y=%.2f Z=%.2f\n", newpos.x,  newpos.y, newpos.z);

    Ogre::Vector3 axis;
    Ogre::Degree rot;
    player_orient.ToAngleAxis(rot, axis);
    printf("Player orient: X=%.2f Y=%.2f Z=%.2f w=%.2f\n", axis.x, axis.y, axis.z, rot.valueDegrees());
    mp_cam_node->setPosition(newpos);

    //float offset_rad = acosf(Ogre::Vector3::UNIT_X.dotProduct(lookdir));
    mp_cam_node->setOrientation(Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_X) * Ogre::Quaternion(Ogre::Degree(270), Ogre::Vector3::UNIT_Y));
    ////mp_cam_node->setOrientation(mp_player->getSceneNode()->getOrientation() * Ogre::Quaternion(Ogre::Degree(90), Ogre::Vector3::UNIT_X));
    printf("----------------\n");
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
    case GLFW_KEY_A:
        cout << "Applying" << endl;
        mp_cam_node->lookAt(mp_player->getSceneNode()->getPosition(), Ogre::Node::TransformSpace::TS_WORLD);
        break;
        //default:
        // Ignore
    }
}

/**
 * Calculates m_run_threshold. The joystick is divided like this (cut
 * off at zero; mirror the graphic below zero for the complete
 * picture). GLFW sends values normalised between -1 and 1 as the
 * extreme positions, 0 being the exact null position.
 *
 *     ^ 1
 *     |       player will
 *     |            run
 *     |
 *     +   <--- m_run_threshold
 *     |       player will
 *     |            walk
 *     -  dead zone below (no movement at all)
 *     - 0
 *
 * m_run_threshold is calculated to be at 1/3 of the space
 * above the dead zone.
 */
void DummyScene::calculateJoyZones()
{
    float usable_zone = 1.0f - GameState::instance.config[FREYA].joy_dead_zone;
    m_run_threshold = 0.3333f * usable_zone;
}
