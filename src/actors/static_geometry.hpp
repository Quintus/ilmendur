#ifndef ILMENDUR_STATIC_GEOMETRY_HPP
#define ILMENDUR_STATIC_GEOMETRY_HPP
#include "actor.hpp"

class StaticGeometry: public Actor
{
public:
    StaticGeometry(SceneSystem::Scene& scene, Ogre::SceneNode* p_scene_node);
    virtual ~StaticGeometry();
};

#endif /* ILMENDUR_STATIC_GEOMETRY_HPP */
