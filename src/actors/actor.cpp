#include "actor.hpp"
#include "../physics/physics.hpp"
#include "../scenes/scene.hpp"
#include <iostream>

using namespace std;

Actor::Actor(SceneSystem::Scene& scene)
    : m_scene(scene),
      mp_scene_node(nullptr),
      m_mass(0.0f),
      m_colltype(PhysicsSystem::ColliderType::box)
{
}

Actor::~Actor()
{
    // Normally the actor should be removed from the physics system by
    // the creator first before the actor is destructed, but as a
    // safety measure, do it here as well.
    if (m_scene.getPhysicsEngine().hasActor(this)) {
        m_scene.getPhysicsEngine().removeActor(this);
    }
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
void Actor::setPosition(float x, float y, float z)
{
    mp_scene_node->setPosition(Ogre::Vector3(x, y, z));

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
void Actor::setOrientation(float angle, float ax, float ay, float az)
{
    mp_scene_node->setOrientation(Ogre::Quaternion(Ogre::Degree(angle), Ogre::Vector3(ax, ay, az)));

    if (m_scene.getPhysicsEngine().hasActor(this)) {
        m_scene.getPhysicsEngine().resetActor(this);
    }
}
