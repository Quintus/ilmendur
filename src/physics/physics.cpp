#include "physics.hpp"
#include "physics_converter.hpp"
#include "debug_drawer.hpp"
#include "../actors/actor.hpp"
#include "../actors/static_geometry.hpp"

using namespace std;
using namespace PhysicsSystem;

/// Gravity the world is exposed to, in m/s².
static const float GRAVITY_ACCEL = -9.81f;

/**
 * Internal object for managing the memory bullet associates
 * with a rigid body.
 */
class PhysicsSystem::RigidBody {
    /**
     * Internal callback object used by bullet to indicate an object
     * is transformed due to physics. See bullet manual version 2.83,
     * pp. 20 f.
     */
    class PhysicsMotionState: public btMotionState {
    public:
        PhysicsMotionState(Ogre::SceneNode* p_node);
        virtual void getWorldTransform(btTransform& trans) const;
        virtual void setWorldTransform(const btTransform& trans);
    private:
        Ogre::SceneNode* mp_node;
    };

public:
    RigidBody(Ogre::Entity* p_entity, float mass, ColliderType ctype);
    ~RigidBody();

    ColliderType          m_colltype;
    btCollisionShape*     mp_bullet_collshape;
    PhysicsMotionState*   mp_bullet_motionstate;
    btRigidBody*          mp_bullet_rbody;
};

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

RigidBody::PhysicsMotionState::PhysicsMotionState(Ogre::SceneNode* p_node)
    : mp_node(p_node)
{
    assert(mp_node);
}

void RigidBody::PhysicsMotionState::getWorldTransform(btTransform& trans) const
{
    trans.setRotation(ogreQuat2Bullet(mp_node->getOrientation()));
    trans.setOrigin(ogreVec2Bullet(mp_node->getPosition()));
}

void RigidBody::PhysicsMotionState::setWorldTransform(const btTransform& trans)
{
    /* Note: Do NOT use Actor::setPosition() or similar functions here --
     * instead apply the transform directly to the Ogre::SceneNode.
     * Otherwise this would cause a call to PhysicsEngine::moveActor(),
     * duplicating the operation. Bullet already knows where the
     * rigid body is in the physics world. */
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

    mp_bullet_motionstate = new RigidBody::PhysicsMotionState(p_entity->getParentSceneNode());

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
                                       p_actor->getColliderType());
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

bool PhysicsEngine::hasActor(Actor* p_actor)
{
    return m_actors.count(p_actor) != 0;
}

/**
 * Resync the actor's position and orientation in bullet's world with
 * those in the Ogre world.
 *
 * This method completely ignores physics. It forcibly resets
 * position and orientation so that it matches whatever is
 * current in the Ogre scene graph. Use this method sparingly.
 * Specifically, this method is not needed for kinematic rigid bodies,
 * because Bullet pulls the world transform for them every frame on
 * itself (see Bullet manual version 2.83, p. 22). Static rigid bodies
 * may not be moved at all (Bullet manual version 2.83, p. 19 f.). So
 * this method is only useful for dynamic rigid bodies. For those,
 * the repositioning methods of Actor automatically call this method.
 */
void PhysicsEngine::resetActor(Actor* p_actor)
{
    RigidBody* p_rbody = m_actors[p_actor];

    // Zero-mass rigid bodies may only be moved if they have been flagged
    // as kinematic, see Bullet manual v 2.83, pp. 19 f. and 22.
    assert(p_actor->getMass() != 0.0f || ((p_rbody->mp_bullet_rbody->getCollisionFlags() & btCollisionObject::CF_KINEMATIC_OBJECT) == btCollisionObject::CF_KINEMATIC_OBJECT));
    // Please leave this assert in even if the ŕigid body is actually
    // removed and re-added below. There might turn up a better option
    // in the future.

    /* There appears to be no other sensible way to force a resync
     * than to entirely remove the rigid body and re-add it. Simply
     * resetting the world transform is insufficient; it leaves forces
     * in place and even worse, appearently some
     * has-touched-the-ground flag. Resetting a still object resting
     * on the ground into the air makes it float in the air. To
     * circumvent that, do it the brutalist way and remove and re-add
     * the rigid body. */
    removeActor(p_actor);
    addActor(p_actor);
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
     * in the RigidBody::PhysicsMotionState object when it is necessary
     * to transform an object due to physics. This is more efficient
     * then iterating over m_actors here. */

    // Now iterate the collisions found and tell the objects about them.
    int num_collisions = m_bullet_colldispatcher.getNumManifolds();
    for(int i=0; i < num_collisions; i++) {
        const btPersistentManifold* p_contact = m_bullet_colldispatcher.getManifoldByIndexInternal(i);
        const btRigidBody* p_obj1 = static_cast<const btRigidBody*>(p_contact->getBody0());
        const btRigidBody* p_obj2 = static_cast<const btRigidBody*>(p_contact->getBody1());

        Actor* p_actor1 = reinterpret_cast<Actor*>(p_obj1->getUserPointer());
        Actor* p_actor2 = reinterpret_cast<Actor*>(p_obj2->getUserPointer());

        p_actor1->collide(*p_actor2);
        p_actor2->collide(*p_actor1);

        // Bullet offers to read the collision points on both objects
        // via p_contact->getContactPoint(), but that's not needed for now.
    }

    mp_debug_drawer->update();
}

/**
 * Apply a force to the actor.
 *
 * \param[in] p_actor
 * Actor to apply the force to.
 *
 * \param[in] force
 * Force vector. To give you a measure: to lift a typical
 * human figure upwards in Ilmendur, you'll need to apply at least
 * about 100 force units into positive Z direction.
 *
 * \param[in] offset
 * The force is normally applied to the actor's mesh's origin.
 * By specifying this parameter, you can apply the force at a different
 * position -- this will result in the actor gaining spin into the
 * respective direction. Normally that probably is not what you want.
 *
 * \remark Ilmendur's Z axis points upwards from the ground. That is,
 * the coordinate system is equivalent to Blender's rather than Ogre's
 * default one, which has the Z axis pointing to the viewer.
 */
void PhysicsEngine::applyForce(Actor* p_actor, const Ogre::Vector3& force, const Ogre::Vector3& offset)
{
    m_actors[p_actor]->mp_bullet_rbody->applyForce(ogreVec2Bullet(force), ogreVec2Bullet(offset));

    /* For performance reasons, Bullet does not include rigid bodies
     * into its calculations once they have come to a rest. They are
     * set to sleep state. Calling activate() disables the sleep
     * state once and makes Bullet check again.
     * See <https://gamedev.stackexchange.com/a/70618> .*/
    m_actors[p_actor]->mp_bullet_rbody->activate();
}
