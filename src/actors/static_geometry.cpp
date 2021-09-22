#include "static_geometry.hpp"
#include <cassert>

StaticGeometry::StaticGeometry(Ogre::SceneNode* p_node)
{
    assert(p_node);

    m_mass = 0.0f;
    mp_scene_node = p_node; // DEBUG! FIXME!
}

StaticGeometry::~StaticGeometry()
{
}
