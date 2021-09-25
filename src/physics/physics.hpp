#ifndef ILMENDUR_PHYSICS_HPP
#define ILMENDUR_PHYSICS_HPP
#include <map>
#include <chrono>
#include <BtOgre.h>

class Actor;
class StaticGeometry;

namespace PhysicsSystem {
    /**
     * This class encapsulates talking to the Bullet physics engine.
     * If your scene needs physics, you want to use it.
     */
    class PhysicsEngine {
    public:
        PhysicsEngine(Ogre::SceneNode* p_root_node);
        ~PhysicsEngine();

        void addActor(Actor* p_actor);
        void addStaticGeometry(StaticGeometry* p_geometry);
        void removeActor(Actor* p_actor);
        void clear();

        void update();
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_last_update;
        BtOgre::DynamicsWorld m_bto_world;
        BtOgre::DebugDrawer m_bto_debug;
    };
}

#endif /* ILMENDUR_PHYSICS_HPP */
