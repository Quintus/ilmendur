#include "static_geometry.hpp"
#include "../physics/physics.hpp"

StaticGeometry::StaticGeometry(SceneSystem::Scene& scene, Ogre::SceneNode* p_scene_node)
    : Actor(scene, p_scene_node)
{
    m_mass = 0.0f;
    m_colltype = PhysicsSystem::ColliderType::trimesh;
    mp_rigid_body = new PhysicsSystem::RigidBody(this);
}

StaticGeometry::~StaticGeometry()
{
}
