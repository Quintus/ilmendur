#include "static_geometry.hpp"
#include "../physics/physics.hpp"

StaticGeometry::StaticGeometry(SceneSystem::Scene& scene, Ogre::SceneNode* p_scene_node)
    : Actor(scene, p_scene_node)
{
    addRigidBody(0.0f, PhysicsSystem::ColliderType::trimesh);
}

StaticGeometry::~StaticGeometry()
{
}
