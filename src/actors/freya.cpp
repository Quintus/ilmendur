#include "freya.hpp"

Freya::Freya(Ogre::SceneNode* p_node)
{
    m_mass = 0.63f;
    mp_scene_node = p_node; // DEBUG! FIXME!
}

Freya::~Freya()
{
}
