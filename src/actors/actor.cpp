#include "actor.hpp"
#include "../physics/physics.hpp"
#include "../scenes/scene.hpp"
#include <iostream>

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
    // Normally the actor should be removed from the physics system by
    // the creator first before the actor is destructed, but as a
    // safety measure, do it here as well.
    if (m_scene.getPhysicsEngine().hasActor(this)) {
        m_scene.getPhysicsEngine().removeActor(this);
    }

    m_scene.getSceneManager().getRootSceneNode()->removeChild(mp_scene_node);
}

void Actor::collide(Actor& other)
{
    cout << this << " collided with " << &other << endl;
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
void Actor::setPosition(const Ogre::Vector3& newpos)
{
    mp_scene_node->setPosition(newpos);

    if (m_scene.getPhysicsEngine().hasActor(this)) {
        m_scene.getPhysicsEngine().resetActor(this);
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
void Actor::setOrientation(const Ogre::Quaternion& newrot)
{
    mp_scene_node->setOrientation(newrot);

    if (m_scene.getPhysicsEngine().hasActor(this)) {
        m_scene.getPhysicsEngine().resetActor(this);
    }
}

/**
 * Both of setPosition() and setOrientation() in one method. For
 * physics-enabled actors, this is more efficient than calling both
 * functions in a row, because the physics engine will be updated only
 * once rather than twice.
 */
void Actor::reposition(const Ogre::Vector3& newpos, const Ogre::Quaternion& newrot)
{
    mp_scene_node->setPosition(newpos);
    mp_scene_node->setOrientation(newrot);

    if (m_scene.getPhysicsEngine().hasActor(this)) {
        m_scene.getPhysicsEngine().resetActor(this);
    }
}

Ogre::Vector3 Actor::getPosition() const
{
    return mp_scene_node->getPosition();
}

Ogre::Quaternion Actor::getOrientation() const
{
    return mp_scene_node->getOrientation();
}
