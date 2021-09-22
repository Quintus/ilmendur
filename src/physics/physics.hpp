#ifndef ILMENDUR_PHYSICS_HPP
#define ILMENDUR_PHYSICS_HPP
#include <map>
#include <chrono>
#include <OGRE/Ogre.h>
#include <btBulletDynamicsCommon.h>

class Actor;

namespace PhysicsSystem {
    /**
     * Internal object for managing memory associated by bullet with
     * an actor.
     */
    struct BulletActorInfo {
        btBoxShape* collshape;
        btDefaultMotionState* motionstate;
        btRigidBody* rigidbody;
    };

    /**
     * This class encapsulates talking to the Bullet physics engine.
     * If your scene needs physics, you want to use it.
     */
    class PhysicsEngine {
    public:
        PhysicsEngine();
        ~PhysicsEngine();

        void addActor(Actor* p_actor);
        void removeActor(Actor* p_actor);
        void clear();

        void update();
    private:
        std::map<Actor*, BulletActorInfo> mp_actors;
        std::chrono::time_point<std::chrono::high_resolution_clock> m_last_update;

        btDefaultCollisionConfiguration* mp_bullet_collconf;
        btCollisionDispatcher* mp_bullet_colldispatcher;
        btBroadphaseInterface* mp_bullet_broadphase;
        btSequentialImpulseConstraintSolver* mp_bullet_solver;
        btDiscreteDynamicsWorld* mp_bullet_world;
    };

    btVector3 ogrevec2btvec(const Ogre::Vector3& vec3);
    Ogre::Vector3 btvec2ogrevec(const btVector3& vec3);
}

#endif /* ILMENDUR_PHYSICS_HPP */
