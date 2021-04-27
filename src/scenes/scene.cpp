#include "scene.hpp"
#include <OGRE/Ogre.h>

using namespace SceneSystem;

Scene::Scene(const std::string& name)
    : mp_scene_manager(Ogre::Root::getSingleton().createSceneManager()),
      m_name(name),
      m_finish(false)
{
}

Scene::~Scene()
{
    Ogre::Root::getSingleton().destroySceneManager(mp_scene_manager);
    mp_scene_manager = nullptr;
}

void Scene::finish()
{
    m_finish = true;
}

bool Scene::isFinishing()
{
    return m_finish;
}
