#include "actor.hpp"
#include "../physics/physics.hpp"
#include "../scenes/scene.hpp"

using namespace std;

/**
 * Creates a new actor for the given scene. If `p_scene_node` is
 * given, the Actor is "wrapped around" that Ogre scene node. If
 * `p_scene_node` is not given, a new empty Ogre::SceneNode is created
 * for the given scene automatically.
 *
 * Note that on destruction, the used Ogre::SceneNode will be detached
 * from the Ogre scene regardless of whether `p_scene_node` was manually
 * specified here or not.
 */
Actor::Actor(SceneSystem::Scene& scene, Ogre::SceneNode* p_scene_node)
    : m_scene(scene),
      mp_rigid_body(nullptr),
      mp_scene_node(p_scene_node ? p_scene_node : m_scene.getSceneManager().getRootSceneNode()->createChildSceneNode()),
      m_mass(0.0f),
      m_colltype(PhysicsSystem::ColliderType::box)
{
    // Ensure the scene node really belongs to the passed scene
    // if it was manually specified.
    if (p_scene_node) {
        assert(&scene.getSceneManager() == p_scene_node->getCreator());
    }
}

Actor::~Actor()
{
    if (mp_rigid_body) {
        delete mp_rigid_body;
    }

    m_scene.getSceneManager().getRootSceneNode()->removeChild(mp_scene_node);
}

void Actor::collide(Actor& other)
{
}

/**
 * This function forcibly resets the actor to the given
 * position. That is, it circumvents the physics engine
 * in case this actor is subject to physics. If it is,
 * the physics engine is told about the updated position.
 *
 * For physics-enabled actors, it is often better to use the physics
 * engine usually by applying forces to the actor.
 */
void Actor::setPosition(const Ogre::Vector3& newpos, bool clear_forces)
{
    mp_scene_node->setPosition(newpos);
    if (mp_rigid_body) {
        mp_rigid_body->reset(clear_forces);
    }
}

/**
 * This function forcibly resets the actor to the given
 * orientation. That is, it circumvents the physics engine
 * in case this actor is subject to physics. If it is,
 * the physics engine is told about the updated orientation.
 *
 * For physics-enabled actors, it is often better to use the physics
 * engine usually by applying forces to the actor.
 */
void Actor::setOrientation(const Ogre::Quaternion& newrot, bool clear_forces)
{
    mp_scene_node->setOrientation(newrot);

    if (mp_rigid_body) {
        mp_rigid_body->reset(clear_forces);
    }
}

/**
 * Both of setPosition() and setOrientation() in one method. For
 * physics-enabled actors, this is more efficient than calling both
 * functions in a row, because the physics engine will be updated only
 * once rather than twice.
 */
void Actor::reposition(const Ogre::Vector3& newpos, const Ogre::Quaternion& newrot, bool clear_forces)
{
    mp_scene_node->setPosition(newpos);
    mp_scene_node->setOrientation(newrot);

    if (mp_rigid_body) {
        mp_rigid_body->reset(clear_forces);
    }
}

/**
 * Adds a rigid body to this actor, i.e., makes it participate in the physics
 * of the current scene. Only call this in scenes which have the physics engine
 * enabled, otherwise this function will crash with an assertion failure.
 *
 * \param mass
 * Object mass in kg. Setting this to zero (0.0f) will create a Bullet
 * static rigid body, which generates collisions, but may not move.
 * If you want to make a kinematic rigid body, call
 * PhysicsEngine::RigidBody::setKinematic() on the returned object.
 * Refer to the Bullet manual for the distinction of dynamic, static,
 * and kinematic rigid bodies.
 *
 * \param colltype
 * This determines how the bounding “box” is calculated. If in doubt,
 * use PhysicsSystem::ColliderType::box, which makes the physics engine
 * use a traditional bounding box. Other shapes are possible, see
 * the documentation of PhysicsSystem::ColliderType.
 *
 * \returns
 * The generated rigid body. This is the same object that is returned
 * by getRigidBody() after this method has completed. It is owned
 * by Actor and destroyed either in the destructor or in removeRigidBody(),
 * so do not delete it yourself.
 *
 * \remark
 * From the point on that this method has returned, the Actor instance
 * is subject to physics, which includes gravity. Consequently, it will
 * immediately start moving downwards until it comes to a rest on something.
 */
PhysicsSystem::RigidBody& Actor::addRigidBody(float mass, PhysicsSystem::ColliderType colltype)
{
    assert(!mp_rigid_body); // May only be called once

    m_mass = mass;
    m_colltype = colltype;
    mp_rigid_body = new PhysicsSystem::RigidBody(this);
    return *mp_rigid_body;
}

void Actor::removeRigidBody()
{
    assert(mp_rigid_body); // Only call after addRigidBody()

    m_mass = 0.0f;
    m_colltype = PhysicsSystem::ColliderType::box;
    delete mp_rigid_body;
    mp_rigid_body = nullptr;
}
