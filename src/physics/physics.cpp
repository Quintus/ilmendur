#include "physics.hpp"
#include "physics_converter.hpp"
#include "debug_drawer.hpp"
#include "../actors/actor.hpp"
#include "../actors/static_geometry.hpp"
#include "../scenes/scene.hpp"

using namespace std;
using namespace PhysicsSystem;

/// Gravity the world is exposed to, in m/s².
static const float GRAVITY_ACCEL = -9.81f;

class RigidBody::PhysicsMotionState: public btMotionState {
public:
    PhysicsMotionState(Ogre::SceneNode* p_node);
    virtual void getWorldTransform(btTransform& trans) const;
    virtual void setWorldTransform(const btTransform& trans);
private:
    Ogre::SceneNode* mp_node;
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

RigidBody::RigidBody(Actor* p_actor)
    : mp_actor(p_actor),
      mp_bullet_collshape(nullptr),
      mp_bullet_motionstate(nullptr),
      mp_bullet_rbody(nullptr)
{
    // Adding a rigid body to an actor makes only sense if the actor is in a scene
    // which has physics enabled. ItÄs up to the caller to not instanciate RigidBody
    // in non-physics scenes.
    assert(mp_actor->getScene().getPhysicsEngine());

    // An actor to be added to the physics universe must have one and only one
    // attached mesh.
    assert(mp_actor->getSceneNode());
    assert(mp_actor->getSceneNode()->numAttachedObjects() == 1);

    Ogre::Entity* p_entity = static_cast<Ogre::Entity*>(mp_actor->getSceneNode()->getAttachedObject(0));
    assert(p_entity->getParentSceneNode()); // Must be attached to the scene
    mp_bullet_collshape = calculateCollisionShape(p_entity, mp_actor->getColliderType());

    btVector3 local_inertia(0, 0, 0);
    if (mp_actor->getMass() != 0.0f) { // zero mass means immobile rigid body to bullet
        mp_bullet_collshape->calculateLocalInertia(mp_actor->getMass(), local_inertia);
    }

    // Bullet calls into this object when it wants to move the Actor
    // or wants to reset the rigid body to the 3D engine's thinking.
    mp_bullet_motionstate = new RigidBody::PhysicsMotionState(mp_actor->getSceneNode());

    // Construct the actual Bullet rigid body
    btRigidBody::btRigidBodyConstructionInfo args(mp_actor->getMass(), mp_bullet_motionstate, mp_bullet_collshape, local_inertia);
    mp_bullet_rbody = new btRigidBody(args);
    mp_bullet_rbody->setUserPointer(mp_actor);
    mp_actor->getScene().getPhysicsEngine()->getBulletWorld()->addRigidBody(mp_bullet_rbody);
}

RigidBody::~RigidBody()
{
    assert(mp_actor->getScene().getPhysicsEngine()); // Physics engine must not be destroyed before the RigidBody!
    mp_actor->getScene().getPhysicsEngine()->getBulletWorld()->removeRigidBody(mp_bullet_rbody);
    delete mp_bullet_rbody;
    delete mp_bullet_motionstate;
    delete mp_bullet_collshape;
}

/**
 * Forcibly resync the rigid body's position and orientation in bullet's world with
 * those in the Ogre world.
 *
 * \param[in] clear_forces (default: false)
 * If set to true, additionally clears all forces on the
 * rigid body. With this set to false, if you reset a moving
 * object, it would continue moving at the new position.
 *
 * \remark This method completely ignores physics. It forcibly resets
 * position and orientation so that it matches whatever is
 * current in the Ogre scene graph. Use this method sparingly.
 * Specifically, this method is not needed for kinematic rigid bodies,
 * because Bullet pulls the world transform for them every frame on
 * itself (see Bullet manual version 2.83, p. 22). Static rigid bodies
 * may not be moved at all (Bullet manual version 2.83, p. 19 f.). So
 * this method is only useful for dynamic rigid bodies.
 */
void RigidBody::reset(bool clear_forces)
{
    // Zero-mass rigid bodies may only be moved if they have been flagged
    // as kinematic, see Bullet manual v 2.83, pp. 19 f. and 22.
    assert(mp_actor->getMass() != 0.0f || ((mp_bullet_rbody->getCollisionFlags() & btCollisionObject::CF_KINEMATIC_OBJECT) == btCollisionObject::CF_KINEMATIC_OBJECT));

    // Bullet has no method to forcibly re-read the world transform
    // appearently, so do it manually.
    btTransform trans;
    mp_bullet_motionstate->getWorldTransform(trans);
    mp_bullet_rbody->setWorldTransform(trans);

    if (clear_forces) {
        mp_bullet_rbody->clearForces();
        mp_bullet_rbody->setLinearVelocity(btVector3(0, 0, 0));
        mp_bullet_rbody->setAngularVelocity(btVector3(0, 0, 0));
    }

    mp_bullet_rbody->activate(); // Wake up the object so bullet notices it has changed
}

/**
 * Apply a force to the rigid body.
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
void RigidBody::applyForce(const Ogre::Vector3& force, const Ogre::Vector3& offset)
{
    mp_bullet_rbody->applyForce(ogreVec2Bullet(force), ogreVec2Bullet(offset));

    /* For performance reasons, Bullet does not include rigid bodies
     * into its calculations once they have come to a rest. They are
     * set to sleep state. Calling activate() disables the sleep
     * state once and makes Bullet check again.
     * See <https://gamedev.stackexchange.com/a/70618> .*/
    mp_bullet_rbody->activate();
}

/**
 * Magically sets the rigid body's velocity to the given one, skipping
 * the need to call applyForce() with an appropriate value.
 */
void RigidBody::setVelocity(const Ogre::Vector3& velocity)
{
    mp_bullet_rbody->setLinearVelocity(ogreVec2Bullet(velocity));
    mp_bullet_rbody->activate(); // See applyForce() for a comment on this
}

/**
 * Magically sets the rigid body's velocity to the given one, skipping
 * the need to call applyForce() with an appropriate value.
 * This variant takes a Vector2 rather than a Vector3. It leaves
 * the Z velocity (i.e. the velocity caused by gravity) alone.
 */
void RigidBody::setVelocity(const Ogre::Vector2& velocity)
{
    btVector3 currvel = mp_bullet_rbody->getLinearVelocity();

    mp_bullet_rbody->setLinearVelocity(btVector3(velocity.x, velocity.y, currvel.z()));
    mp_bullet_rbody->activate(); // See applyForce() for a comment on this
}

/**
 * Exempts this rigid body from having changed its orientation by
 * physics. Any rotation will thus have to be conducted manually. This
 * is particularly useful for characters, which you'd typically not
 * want to fall sideways like a static object.
 *
 * \remark This method implements the suggestion from p. 26 of v. 2.83 of
 * the Bullet Manual (i.e. uses setAngularFactor()).
 */
void RigidBody::lockRotation()
{
    mp_bullet_rbody->setAngularFactor(btVector3(0, 0, 0));
}

/**
 * Make a static rigid body into a kinematic rigid body or a kinematic rigid
 * body into a static rigid body. As per Bullet manual version 2.83 p. 20,
 * kinematic rigid bodies are a special kind of static rigid body which
 * contrary to normal static rigid bodies is allowed to be moved by the
 * caller manually. It will never be moved by Bullet, though. Kinematic
 * rigid bodies must have zero mass (ibid.). This method crashes with
 * an assertion failure if this assertion is violated.
 *
 * \param val
 * Whether to turn on (true) or off (false) the kinematic attribute.
 *
 * \remark
 * As recommended by the Bullet manual version 2.83, p. 22, this method
 * automatically sets DISABLE_ACTIVATION on the object when the kinematic
 * attribute is being enabled (and turns it off when it is being disbaled).
 */
void RigidBody::setKinematic(bool val)
{
    assert(mp_actor->getMass() == 0.0f); // Kinematic rigid bodies must have zero mass as per Bullet manual

    if (val) {
        mp_bullet_rbody->setCollisionFlags(mp_bullet_rbody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
        mp_bullet_rbody->setActivationState(DISABLE_DEACTIVATION);
    } else {
        mp_bullet_rbody->setCollisionFlags(mp_bullet_rbody->getCollisionFlags() ^ btCollisionObject::CF_KINEMATIC_OBJECT);
        mp_bullet_rbody->setActivationState(0); // FIXME: Undocumented, untested, does this work?
    }
}

/**
 * Returns true if this is a static rigid body (= zero mass) which has the
 * kinematic flag (see setKinematic()) set. Otherwise returns false.
 */
bool RigidBody::getKinematic()
{
    return mp_actor->getMass() == 0.0f && ((mp_bullet_rbody->getCollisionFlags() & btCollisionObject::CF_KINEMATIC_OBJECT) == btCollisionObject::CF_KINEMATIC_OBJECT);
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
    delete mp_debug_drawer;
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
        const btPersistentManifold* p_manifold = m_bullet_colldispatcher.getManifoldByIndexInternal(i);
        // These presence of a manifold is not sufficient to know if there is a collision.
        // It is required to check if there are actually contacts.
        if (p_manifold->getNumContacts() > 0) {
            const btRigidBody* p_obj1 = static_cast<const btRigidBody*>(p_manifold->getBody0());
            const btRigidBody* p_obj2 = static_cast<const btRigidBody*>(p_manifold->getBody1());

            Actor* p_actor1 = reinterpret_cast<Actor*>(p_obj1->getUserPointer());
            Actor* p_actor2 = reinterpret_cast<Actor*>(p_obj2->getUserPointer());

            p_actor1->collide(*p_actor2);
            p_actor2->collide(*p_actor1);

            // Bullet offers to read the collision points on both objects
            // via p_manifold->getContactPoint(), but that's not needed for now.
        }
    }

    mp_debug_drawer->update();
}
