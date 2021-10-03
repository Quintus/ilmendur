#ifndef ILMENDURFREYA_HPP
#define ILMENDURFREYA_HPP
#include "actor.hpp"

class Freya: public Actor
{
public:
    Freya(SceneSystem::Scene& scene, Ogre::SceneNode* p_node);
    virtual ~Freya();
};

#endif /* ILMENDURFREYA_HPP */
