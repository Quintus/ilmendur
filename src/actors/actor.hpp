#ifndef ILMENDURACTOR_HPP
#define ILMENDURACTOR_HPP

namespace Ogre {
    class SceneNode;
}

namespace PhysicsSystem {
    enum class ColliderType;
}

namespace SceneSystem {
    class Scene;
}

class Actor {
public:
    Actor(SceneSystem::Scene& scene);
    virtual ~Actor();

    inline float getMass() const { return m_mass; }
    inline PhysicsSystem::ColliderType getColliderType() const { return m_colltype; }

    virtual void collide(Actor& other);

    void setPosition(float x, float y, float z);

    inline Ogre::SceneNode* getSceneNode() const { return mp_scene_node; }
protected:
    SceneSystem::Scene& m_scene;
    Ogre::SceneNode* mp_scene_node;
    float m_mass;
    PhysicsSystem::ColliderType m_colltype;
};

#endif /* ILMENDURACTOR_HPP */
