#include "freya.hpp"
#include "../scenes/scene.hpp"
#include "../physics/physics.hpp"

Freya::Freya(SceneSystem::Scene& scene)
    : Actor(scene)
{
    //mp_scene_node->attachObject(m_scene.getSceneManager().createEntity("sign.mesh"));
    getSceneNode()->attachObject(getScene().getSceneManager().createEntity(Ogre::SceneManager::PrefabType::PT_CUBE));
    getSceneNode()->setScale(0.01, 0.01, 0.01);

    PhysicsSystem::RigidBody& rbody = addRigidBody(0.63f, PhysicsSystem::ColliderType::box);
    rbody.lockRotation();
}

Freya::~Freya()
{
}
