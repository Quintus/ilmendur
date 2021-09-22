#ifndef ILMENDURACTOR_HPP
#define ILMENDURACTOR_HPP

namespace Ogre {
    class SceneNode;
}

class Actor {
public:
    Actor();
    virtual ~Actor();

    inline float getMass() const { return m_mass; }
    inline Ogre::SceneNode* getSceneNode() const { return mp_scene_node; }
protected:
    float m_mass;
    Ogre::SceneNode* mp_scene_node;
};

#endif /* ILMENDURACTOR_HPP */
