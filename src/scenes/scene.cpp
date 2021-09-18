#include "scene.hpp"
#include "../entities/entity.hpp"
#include <OGRE/Ogre.h>
#include <cassert>
#include <iostream>

using namespace SceneSystem;
using namespace std;

Scene::Scene(const string& name)
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

/**
 * This function filters all scene nodes starting at `p_start` for
 * nodes which have the `il_entity` property set (this is set in
 * Blender). It replaces those nodes according to the information in
 * the entity_type table (see entities/entity.hpp).
 *
 * `p_node` itself is also checked.
 */
void Scene::replaceBlenderEntities(Ogre::Node* p_node)
{
    Ogre::SceneNode* p_scene_node = static_cast<Ogre::SceneNode*>(p_node);
    cout << "Executing replaceBlanderEntities() for " << p_scene_node->getName() << endl;

    if (p_scene_node->numAttachedObjects() > 0) { // Transient nodes have zero entities
        assert(p_scene_node->numAttachedObjects() == 1); // blender2ogre always exports 1 node per entity
        Ogre::MovableObject* p_entity = p_scene_node->getAttachedObject(0);

        if (p_entity->getUserObjectBindings().getUserAny("il_entity").has_value()) {
            int etype = Ogre::any_cast<int>(p_entity->getUserObjectBindings().getUserAny("il_entity"));
            cout << "il_entity type " << etype << " found! This needs to be replaced here" << endl;
            return;
        }
    }

    for(Ogre::Node* p_child: p_scene_node->getChildren()) {
        replaceBlenderEntities(p_child);
    }

    cout << "replaceBlanderEntities() done for " << p_scene_node->getName() << endl;
}
