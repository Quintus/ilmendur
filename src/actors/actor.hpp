#ifndef ILMENDURACTOR_HPP
#define ILMENDURACTOR_HPP
#include <OGRE/Ogre.h>
#include "../physics/physics.hpp"

namespace PhysicsSystem {
    enum class ColliderType;
}

namespace SceneSystem {
    class Scene;
}

class Actor {
public:
    Actor(SceneSystem::Scene& scene, Ogre::SceneNode* p_scene_node = nullptr);
    virtual ~Actor();

    inline float getMass() const { return m_mass; }
    inline PhysicsSystem::ColliderType getColliderType() const { return m_colltype; }

    virtual void collide(Actor& other);

    void setPosition(const Ogre::Vector3& newpos, bool clear_forces = false);
    void setOrientation(const Ogre::Quaternion& neworient, bool clear_forces = false);
    void reposition(const Ogre::Vector3& newpos, const Ogre::Quaternion& neworient, bool clear_forces = false);

    inline SceneSystem::Scene& getScene() const { return m_scene; }
    inline Ogre::SceneNode* getSceneNode() const { return mp_scene_node; }
    inline PhysicsSystem::RigidBody* getRigidBody() const { return mp_rigid_body; }

protected:
    SceneSystem::Scene& m_scene;
    PhysicsSystem::RigidBody* mp_rigid_body;
    Ogre::SceneNode* mp_scene_node;
    float m_mass;
    PhysicsSystem::ColliderType m_colltype;
};

#endif /* ILMENDURACTOR_HPP */
