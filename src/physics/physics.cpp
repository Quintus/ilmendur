#include "physics.hpp"
#include "physics_converter.hpp"
#include "debug_drawer.hpp"
#include "../actors/actor.hpp"
#include "../actors/static_geometry.hpp"

using namespace std;
using namespace PhysicsSystem;

/// Gravity the world is exposed to, in m/s².
static const float GRAVITY_ACCEL = -9.81f;

btQuaternion PhysicsSystem::ogreQuat2Bullet(const Ogre::Quaternion& q)
{
    return btQuaternion(q.x, q.y, q.z, q.w);
}

btVector3 PhysicsSystem::ogreVec2Bullet(const Ogre::Vector3& v)
{
    return btVector3(v.x, v.y, v.z);
}

Ogre::Quaternion PhysicsSystem::bulletQuat2Ogre(const btQuaternion& q)
{
    return Ogre::Quaternion(q.w(), q.x(), q.y(), q.z());
}

Ogre::Vector3 PhysicsSystem::bulletVec2Ogre(const btVector3& v)
{
    return Ogre::Vector3(v.x(), v.y(), v.z());
}

PhysicsMotionState::PhysicsMotionState(Ogre::SceneNode* p_node)
    : mp_node(p_node)
{
    assert(mp_node);
}

void PhysicsMotionState::getWorldTransform(btTransform& trans) const
{
    trans.setRotation(ogreQuat2Bullet(mp_node->getOrientation()));
    trans.setOrigin(ogreVec2Bullet(mp_node->getPosition()));
}

void PhysicsMotionState::setWorldTransform(const btTransform& trans)
{
    mp_node->setOrientation(bulletQuat2Ogre(trans.getRotation()));
    mp_node->setPosition(bulletVec2Ogre(trans.getOrigin()));
}

RigidBody::RigidBody(Ogre::Entity* p_entity, float mass, ColliderType ctype)
    : m_colltype(ctype),
      mp_bullet_collshape(nullptr),
      mp_bullet_motionstate(nullptr),
      mp_bullet_rbody(nullptr)
{
    assert(p_entity->getParentSceneNode()); // Must be attached to the scene
    mp_bullet_collshape = calculateCollisionShape(p_entity, ctype);

    btVector3 local_inertia(0, 0, 0);
    if (mass != 0.0f) { // zero mass means immobile rigid body to bullet
        mp_bullet_collshape->calculateLocalInertia(mass, local_inertia);
    }

    mp_bullet_motionstate = new PhysicsMotionState(p_entity->getParentSceneNode());

    btRigidBody::btRigidBodyConstructionInfo args(mass, mp_bullet_motionstate, mp_bullet_collshape, local_inertia);
    mp_bullet_rbody = new btRigidBody(args);
}

RigidBody::~RigidBody()
{
    delete mp_bullet_rbody;
    delete mp_bullet_motionstate;
    delete mp_bullet_collshape;
}

PhysicsEngine::PhysicsEngine(Ogre::SceneNode* p_root_node)
    : m_bullet_colldispatcher(&m_bullet_collconfig),
      m_bullet_world(&m_bullet_colldispatcher, &m_bullet_broadphase, &m_bullet_solver, &m_bullet_collconfig),
      m_last_update(chrono::high_resolution_clock::now()),
      mp_debug_drawer(new DebugDrawer(p_root_node, &m_bullet_world))
{
    m_bullet_world.setGravity(btVector3(0, 0, GRAVITY_ACCEL));
}

PhysicsEngine::~PhysicsEngine()
{
    clear();
    delete mp_debug_drawer;
}

void PhysicsEngine::addActor(Actor* p_actor)
{
    // An actor to be added to the physics universe must have one and only one
    // attached mesh. It must not already have been added.
    assert(p_actor->getSceneNode()->numAttachedObjects() == 1);
    assert(m_actors.count(p_actor) == 0);

    RigidBody* p_rbody = new RigidBody(static_cast<Ogre::Entity*>(p_actor->getSceneNode()->getAttachedObject(0)),
                                       p_actor->getMass(),
                                       ColliderType::box);
    p_rbody->mp_bullet_rbody->setUserPointer(p_actor);
    m_actors[p_actor] = p_rbody;
    m_bullet_world.addRigidBody(p_rbody->mp_bullet_rbody);
}

void PhysicsEngine::removeActor(Actor* p_actor)
{
    assert(m_actors.count(p_actor) == 1);

    RigidBody* p_rbody = m_actors[p_actor];
    m_bullet_world.removeRigidBody(p_rbody->mp_bullet_rbody);
    m_actors.erase(p_actor);
    delete p_rbody;
}

void PhysicsEngine::addStaticGeometry(StaticGeometry* p_geometry)
{
    // An actor to be added to the physics universe must have one and only one
    // attached mesh.
    assert(p_geometry->getSceneNode()->numAttachedObjects() == 1);
    assert(p_geometry->getMass() == 0.0f); // Bullet mandates zero mass for static objects
    assert(m_actors.count(p_geometry) == 0);

    RigidBody* p_rbody = new RigidBody(static_cast<Ogre::Entity*>(p_geometry->getSceneNode()->getAttachedObject(0)),
                                       p_geometry->getMass(),
                                       ColliderType::trimesh);
    p_rbody->mp_bullet_rbody->setUserPointer(p_geometry);
    m_actors[p_geometry] = p_rbody;
    m_bullet_world.addRigidBody(p_rbody->mp_bullet_rbody);
}

/**
 * Removes all actors from the physics world.
 */
void PhysicsEngine::clear()
{
    while (!m_actors.empty()) {
        removeActor(m_actors.begin()->first);
    }
}

void PhysicsEngine::update()
{
    chrono::time_point<chrono::high_resolution_clock> now = chrono::high_resolution_clock::now();
    m_bullet_world.stepSimulation(chrono::duration_cast<chrono::microseconds>(now - m_last_update).count() / 1000000.0);
    m_last_update = now;

    /* Bullet in stepSimulation() calls the callback functions
     * in the PhysicsMotionState object when it is necessary
     * to transform an object due to physics. This is more efficient
     * then iterating over m_actors here. */

    mp_debug_drawer->update();
}
