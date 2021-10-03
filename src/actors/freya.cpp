#include "freya.hpp"
#include "../physics/physics.hpp"
#include <cassert>

Freya::Freya(SceneSystem::Scene& scene, Ogre::SceneNode* p_node)
    : Actor(scene)
{
    assert(p_node);
    m_mass = 0.63f;
    m_colltype = PhysicsSystem::ColliderType::box;

    mp_scene_node = p_node; // DEBUG! FIXME!
}

Freya::~Freya()
{
}
