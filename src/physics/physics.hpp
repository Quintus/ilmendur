#ifndef ILMENDUR_PHYSICS_HPP
#define ILMENDUR_PHYSICS_HPP
#include <map>
#include <chrono>
#include <btBulletDynamicsCommon.h>
#include <Ogre.h>

class Actor;
class StaticGeometry;

namespace PhysicsSystem {

    class DebugDrawer;

    /**
     * Collision skin types. Any mesh that participates in physics
     * has one of these.
     */
    enum class ColliderType {
        box,     ///< Use a collision box.
        sphere,  ///< Use a collision sphere.
        trimesh, ///< Use an exactly fitting triangle mesh (expensive)
        hull     ///< ?
    };

    /**
     * Internal callback object used by bullet to indicate an object
     * is transformed due to physics. See bullet manual version 2.83,
     * pp. 20 f.
     */
    class PhysicsMotionState: public btMotionState
    {
    public:
        PhysicsMotionState(Ogre::SceneNode* p_node);
        virtual void getWorldTransform(btTransform& trans) const;
        virtual void setWorldTransform(const btTransform& trans);
    private:
        Ogre::SceneNode* mp_node;
    };

    class PhysicsEngine;

    /**
     * Internal object for managing the memory bullet associates
     * with a rigid body.
     */
    class RigidBody {
    public:
        RigidBody(Ogre::Entity* p_entity, float mass, ColliderType ctype);
        ~RigidBody();

        ColliderType          m_colltype;
        btCollisionShape*     mp_bullet_collshape;
        PhysicsMotionState*   mp_bullet_motionstate;
        btRigidBody*          mp_bullet_rbody;
    };

    /**
     * This class encapsulates talking to the Bullet physics engine.
     * If your scene needs physics, you want to use it.
     */
    class PhysicsEngine {
    public:
        PhysicsEngine(Ogre::SceneNode* p_root_node);
        ~PhysicsEngine();

        void addActor(Actor* p_actor);
        void removeActor(Actor* p_actor);
        bool hasActor(Actor* p_actor);
        void moveActor(Actor* p_actor, const Ogre::Vector3& pos);
        void clear();

        void update();
    private:
        btDefaultCollisionConfiguration m_bullet_collconfig;
        btCollisionDispatcher m_bullet_colldispatcher;
        btDbvtBroadphase m_bullet_broadphase;
        btSequentialImpulseConstraintSolver m_bullet_solver;
        btDiscreteDynamicsWorld m_bullet_world;

        std::chrono::time_point<std::chrono::high_resolution_clock> m_last_update;
        std::map<Actor*, RigidBody*> m_actors;
        DebugDrawer* mp_debug_drawer;
    };

    btQuaternion ogreQuat2Bullet(const Ogre::Quaternion& q);
    btVector3    ogreVec2Bullet(const Ogre::Vector3& v);
    Ogre::Quaternion bulletQuat2Ogre(const btQuaternion& q);
    Ogre::Vector3    bulletVec2Ogre(const btVector3& v);
}

#endif /* ILMENDUR_PHYSICS_HPP */
