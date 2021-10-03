#ifndef ILMENDURACTOR_HPP
#define ILMENDURACTOR_HPP
#include <OGRE/Ogre.h>

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

    void setPosition(const Ogre::Vector3& newpos);
    void setOrientation(const Ogre::Quaternion& neworient);
    void reposition(const Ogre::Vector3& newpos, const Ogre::Quaternion& neworient);

    Ogre::Vector3 getPosition() const;
    Ogre::Quaternion getOrientation() const;

    inline Ogre::SceneNode* getSceneNode() const { return mp_scene_node; }
protected:
    SceneSystem::Scene& m_scene;
    Ogre::SceneNode* mp_scene_node;
    float m_mass;
    PhysicsSystem::ColliderType m_colltype;
};

#endif /* ILMENDURACTOR_HPP */
