#ifndef ILMENDUR_STATIC_GEOMETRY_HPP
#define ILMENDUR_STATIC_GEOMETRY_HPP
#include "actor.hpp"

class StaticGeometry: public Actor
{
public:
    StaticGeometry(Ogre::SceneNode* p_node);
    virtual ~StaticGeometry();
};

#endif /* ILMENDUR_STATIC_GEOMETRY_HPP */
