#ifndef ILMENDUR_PHYSICS_HPP
#define ILMENDUR_PHYSICS_HPP
#include <chrono>
#include <btBulletDynamicsCommon.h>
#include <OGRE/Ogre.h>

class Actor;

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
     * Representation of an Actor’s rigid body, i.e. the object
     * physical forces act upon. Under the hood, this class wraps
     * the real Bullet3D btRigidBody object plus the memory of
     * some helper objects associated with the btRigidBody.
     * In addition, the methods it provides do the necessary conversion
     * from Ilmendur/Ogre types to Bullet3d’s types (e.g. Ogre::Vector3
     * to btVector3).
     */
    class RigidBody {
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
        RigidBody(Actor* p_actor);
        ~RigidBody();

        void reset(bool clear_forces = false);
        void lockRotation();
        void applyForce(const Ogre::Vector3& force, const Ogre::Vector3& offset = Ogre::Vector3(0, 0, 0));
        void setVelocity(const Ogre::Vector3& velocity);
        void setVelocity(const Ogre::Vector2& velocity);

    private:
        Actor*                mp_actor;
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

        inline btDiscreteDynamicsWorld* getBulletWorld() { return &m_bullet_world; }

        void update();
    private:
        btDefaultCollisionConfiguration m_bullet_collconfig;
        btCollisionDispatcher m_bullet_colldispatcher;
        btDbvtBroadphase m_bullet_broadphase;
        btSequentialImpulseConstraintSolver m_bullet_solver;
        btDiscreteDynamicsWorld m_bullet_world;

        std::chrono::time_point<std::chrono::high_resolution_clock> m_last_update;
        DebugDrawer* mp_debug_drawer;
    };

    btQuaternion ogreQuat2Bullet(const Ogre::Quaternion& q);
    btVector3    ogreVec2Bullet(const Ogre::Vector3& v);
    Ogre::Quaternion bulletQuat2Ogre(const btQuaternion& q);
    Ogre::Vector3    bulletVec2Ogre(const btVector3& v);
}

#endif /* ILMENDUR_PHYSICS_HPP */
