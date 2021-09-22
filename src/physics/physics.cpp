#include "physics.hpp"
#include "../actors/actor.hpp"

using namespace std;
using namespace PhysicsSystem;

/// Gravity the world is exposed to.
static const float GRAVITY_ACCEL = -10.0f;

/**
 * Converts an Ogre vector to a Bullet vector.
 */
btVector3 PhysicsSystem::ogrevec2btvec(const Ogre::Vector3& vec3)
{
    return btVector3(btScalar(vec3.x), btScalar(vec3.y), btScalar(vec3.z));
}

/**
 * Converts a Bullet vector to an Ogre vector.
 */
Ogre::Vector3 PhysicsSystem::btvec2ogrevec(const btVector3& vec3)
{
    return Ogre::Vector3(vec3.getX(), vec3.getY(), vec3.getZ());
}

PhysicsEngine::PhysicsEngine()
    : mp_bullet_collconf(new btDefaultCollisionConfiguration()),
      mp_bullet_colldispatcher(new btCollisionDispatcher(mp_bullet_collconf)),
      mp_bullet_broadphase(new btDbvtBroadphase()),
      mp_bullet_solver(new btSequentialImpulseConstraintSolver()),
      mp_bullet_world(new btDiscreteDynamicsWorld(mp_bullet_colldispatcher, mp_bullet_broadphase, mp_bullet_solver, mp_bullet_collconf))
{
    mp_bullet_world->setGravity(btVector3(0.0f, 0.0f, GRAVITY_ACCEL));
}

PhysicsEngine::~PhysicsEngine()
{
    assert(mp_actors.empty()); // If this fails, then the bullet world and the ogre scene got out of sync.

    delete mp_bullet_world;
    delete mp_bullet_solver;
    delete mp_bullet_broadphase;
    delete mp_bullet_colldispatcher;
    delete mp_bullet_collconf;
}

void PhysicsEngine::addActor(Actor* p_actor)
{
    // An actor to be added to the physics universe must have one and only one
    // attached mesh.
    assert(p_actor->getSceneNode()->numAttachedObjects() == 1);

    BulletActorInfo binfo;
    binfo.collshape = new btBoxShape(ogrevec2btvec(p_actor->getSceneNode()->getAttachedObject(0)->getBoundingBox().getHalfSize()));
    btVector3 local_inertia(0, 0, 0);
    binfo.collshape->calculateLocalInertia(p_actor->getMass(), local_inertia);

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(ogrevec2btvec(p_actor->getSceneNode()->getPosition()));
    binfo.motionstate = new btDefaultMotionState(transform);

    btRigidBody::btRigidBodyConstructionInfo args(p_actor->getMass(), binfo.motionstate, binfo.collshape, local_inertia);
    binfo.rigidbody = new btRigidBody(args);

    mp_bullet_world->addRigidBody(binfo.rigidbody);
    mp_actors[p_actor] = move(binfo);
    m_last_update = chrono::high_resolution_clock::now();
}

void PhysicsEngine::removeActor(Actor* p_actor)
{
    mp_bullet_world->removeCollisionObject(mp_actors[p_actor].rigidbody);
    delete mp_actors[p_actor].rigidbody;
    delete mp_actors[p_actor].motionstate;
    delete mp_actors[p_actor].collshape;
    mp_actors.erase(p_actor);
}

/**
 * Removes all actors from the physics world.
 */
void PhysicsEngine::clear()
{
    while (!mp_actors.empty()) {
        removeActor(mp_actors.begin()->first);
    }
}

void PhysicsEngine::update()
{
    chrono::time_point<chrono::high_resolution_clock> now = chrono::high_resolution_clock::now();
    mp_bullet_world->stepSimulation(chrono::duration_cast<chrono::microseconds>(now - m_last_update).count() / 1000000.0);
    m_last_update = now;

    for (auto iter: mp_actors) {
        btTransform trans;
        iter.second.motionstate->getWorldTransform(trans);
        iter.first->getSceneNode()->setPosition(btvec2ogrevec(trans.getOrigin()));
    }
}
