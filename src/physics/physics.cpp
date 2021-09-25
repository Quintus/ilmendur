#include "physics.hpp"
#include "../actors/actor.hpp"
#include "../actors/static_geometry.hpp"

using namespace std;
using namespace PhysicsSystem;

/// Gravity the world is exposed to, in m/s².
static const float GRAVITY_ACCEL = -9.81f;

PhysicsEngine::PhysicsEngine(Ogre::SceneNode* p_root_node)
    : m_bto_world(Ogre::Vector3(0, 0, GRAVITY_ACCEL)),
      m_bto_debug(p_root_node, m_bto_world.getBtWorld())
{
    m_last_update = chrono::high_resolution_clock::now();
}

PhysicsEngine::~PhysicsEngine()
{
    m_bto_debug.clear();
}

void PhysicsEngine::addActor(Actor* p_actor)
{
    // An actor to be added to the physics universe must have one and only one
    // attached mesh.
    assert(p_actor->getSceneNode()->numAttachedObjects() == 1);
    m_bto_world.addRigidBody(
        p_actor->getMass(),
        static_cast<Ogre::Entity*>(p_actor->getSceneNode()->getAttachedObject(0)),
        BtOgre::CT_BOX);
}

void PhysicsEngine::removeActor(Actor* p_actor)
{
    // TODO
    /*mp_bullet_world->removeCollisionObject(mp_actors[p_actor].rigidbody);
    delete mp_actors[p_actor].rigidbody;
    delete mp_actors[p_actor].motionstate;
    delete mp_actors[p_actor].collshape;
    mp_actors.erase(p_actor); */
}

void PhysicsEngine::addStaticGeometry(StaticGeometry* p_geometry)
{
    // An actor to be added to the physics universe must have one and only one
    // attached mesh.
    assert(p_geometry->getSceneNode()->numAttachedObjects() == 1);
    assert(p_geometry->getMass() == 0.0f); // Bullet mandates zero mass for static objects

    m_bto_world.addRigidBody(
        p_geometry->getMass(),
        static_cast<Ogre::Entity*>(p_geometry->getSceneNode()->getAttachedObject(0)),
        BtOgre::CT_TRIMESH);
}

/**
 * Removes all actors from the physics world.
 */
void PhysicsEngine::clear()
{
    // TODO
    /*while (!mp_actors.empty()) {
        removeActor(mp_actors.begin()->first);
        }*/
}

void PhysicsEngine::update()
{
    chrono::time_point<chrono::high_resolution_clock> now = chrono::high_resolution_clock::now();
    m_bto_world.getBtWorld()->stepSimulation(chrono::duration_cast<chrono::microseconds>(now - m_last_update).count() / 1000000.0);
    m_last_update = now;

    m_bto_debug.update();

    /* Note: BtOgre registers a callback into Bullet, in which
     * it automatically updates all the scene nodes associated with
     * the rigid bodies, so there is no need to iterate the list
     * of rigid bodies here and reposition the scene notes. */
}
