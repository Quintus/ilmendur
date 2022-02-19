#ifndef ILMENDUR_PHYSICS_HPP
#define ILMENDUR_PHYSICS_HPP
#include <map>
#include <chrono>
#include <btBulletDynamicsCommon.h>
#include <OGRE/Ogre.h>

class Actor;

namespace PhysicsSystem {

    class DebugDrawer;
    class RigidBody;

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
        void resetActor(Actor* p_actor, bool clear_forces = false);
        void clear();

        void lockRotation(Actor* p_actor);
        void applyForce(Actor* p_actor, const Ogre::Vector3& force, const Ogre::Vector3& offset = Ogre::Vector3(0, 0, 0));
        void setVelocity(Actor* p_actor, const Ogre::Vector3& velocity);
        void setVelocity(Actor* p_actor, const Ogre::Vector2& velocity);

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
