#include "static_geometry.hpp"
#include "../physics/physics.hpp"
#include <cassert>

StaticGeometry::StaticGeometry(SceneSystem::Scene& scene, Ogre::SceneNode* p_node)
    : Actor(scene)
{
    assert(p_node);

    m_mass = 0.0f;
    m_colltype = PhysicsSystem::ColliderType::trimesh;
    mp_scene_node = p_node; // DEBUG! FIXME!
}

StaticGeometry::~StaticGeometry()
{
}
