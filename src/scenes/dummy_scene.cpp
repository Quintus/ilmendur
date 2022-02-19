#include "dummy_scene.hpp"
#include "../core/application.hpp"
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
    // but without the player’s rotation. This allows to attach the camera
    // to it and control the camera independently from the player's current
    // rotation.
    mp_camera_target = mp_scene_manager->getRootSceneNode()->createChildSceneNode();

    // also need to tell where we are
    mp_cam_node = mp_camera_target->createChildSceneNode();
    mp_cam_node->setPosition(-5, 0, CAMERA_HEIGHT);
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
    Ogre::Vector3 vec(joyaxes[config.joy_horizontal.axisno], joyaxes[config.joy_vertical.axisno], 0.0f);
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
    if (config.joy_vertical.inverted) {
        vec.y *= -1;
    }
    if (config.joy_horizontal.inverted) {
        vec.x *= -1;
    }

    mp_camera_target->rotate(Ogre::Vector3::UNIT_Z, Ogre::Degree(vec.x * 5.0f));

    return;

    // The goal is to rotate the player figure relative to the camera
    // direction by the offset that `vec' has from the
    // joystick's Y axis (which points from (0|0) upwards to (0|1)):

    // Calculate the angle between the y-axis vector (0, 1) and the read input vector
    // by reversing the scalar product.
    float angle_rad = acosf((vec.y * vec.y) / ((vec.x * vec.x) + (vec.y * vec.y)) );

    // Convert it to a part of the full circle, counter-clockwise
    if (vec.x == 0.0f && vec.y == -1.0f) {
        // Rotating by 360° is nonsense, do not rotate in that case
        angle_rad = 0.0f;
    } else {
        if (vec.x < 0.0f) {
            if (vec.y < 0.0f) {
                // angle_rad = angle_rad // Change nothing
            } else {
                angle_rad = M_PI - angle_rad;
            }
        } else {
            if (vec.y < 0.0f) {
                angle_rad = 2 * M_PI - angle_rad;
            } else {
                angle_rad = M_PI + angle_rad;
            }
        }
    }

    /* At this point, an angle of zero degrees means the player pressed
     * the joystick straight UP, whereas 180 degrees means straight DOWN.
     * 90 degrees is RIGHT, and 270 degrees is LEFT. Beware `angle_rad' is
     * in radians, not in angles. */

    // Get the vector the camera is looking down
    //Ogre::Quaternion cam_orient = mp_cam_node->getOrientation();
    Ogre::Quaternion cam_orient = mp_player->getSceneNode()->getOrientation();
    Ogre::Vector3 lookdir = cam_orient * Ogre::Vector3::UNIT_X;
    lookdir.normalise();

    /* Calculate the angle between `lookdir' and the player model's
     * natural looking direction (which is UNIT_X as per instructions
     * for the artists; also note that the `lookdir' and `UNIT_X'
     * vectors are of length 1, which is quite helpful for the acosf()
     * calculation used below). From that, find the required
     * counter-clockwise rotation required to make the player look
     * into the camera direction. Note that offsets smaller than 180°
     * need to be calculated by subtracting from the full circle. */
    float offset_rad = acosf(Ogre::Vector3::UNIT_X.dotProduct(lookdir));
    if (lookdir.y < 0.0f) {
        offset_rad = 2 * M_PI - offset_rad;
    }
    Ogre::Quaternion player_orient(Ogre::Radian(offset_rad + angle_rad), Ogre::Vector3::UNIT_Z);
    mp_player->getSceneNode()->setOrientation(player_orient);

    Ogre::Vector3 v;
    Ogre::Degree d;
    cam_orient.ToAngleAxis(d, v);
    printf("q=(%+07.2f,%+07.2f,%+07.2f,%+07.2f),l=(%+07.2f|%+07.2f|%+07.2f); o=%+07.2f\n", v.x, v.y, v.z, d.valueDegrees(), lookdir.x, lookdir.y, lookdir.z, Ogre::Radian(offset_rad).valueDegrees());

    return;

    /* Depending on how strong is pressed, move fast or slow forward
     * into this direction. The quaternion `player_orient' is rotating
     * around the Z axis, and UNIT_X is the model's natural looking
     * direction as per instructions for the artists. */
    Ogre::Vector3 translation = player_orient * Ogre::Vector3::UNIT_X;
    translation.normalise();
    if (vec.length() > m_run_threshold) {
        translation *= 0.5;
    }
    else {
        translation *= 0.2;
    }

    mp_player->getSceneNode()->translate(translation);
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
