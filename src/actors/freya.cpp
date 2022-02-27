#include "freya.hpp"
#include "../scenes/scene.hpp"
#include "../physics/physics.hpp"

Freya::Freya(SceneSystem::Scene& scene)
    : Actor(scene)
{
    m_mass = 0.63f;
    m_colltype = PhysicsSystem::ColliderType::box;
    //mp_scene_node->attachObject(m_scene.getSceneManager().createEntity("sign.mesh"));
    mp_scene_node->attachObject(m_scene.getSceneManager().createEntity(Ogre::SceneManager::PrefabType::PT_CUBE));
    mp_scene_node->setScale(0.01, 0.01, 0.01);
}

Freya::~Freya()
{
}
