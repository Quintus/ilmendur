#include "scene.hpp"
#include "../entities/entity.hpp"
#include "../exttexts/exttexts.hpp"
#include <cassert>
#include <iostream>

using namespace SceneSystem;
using namespace std;

Scene::Scene(const string& name)
    : mp_physics(nullptr),
      mp_scene_manager(Ogre::Root::getSingleton().createSceneManager()),
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
 * `p_scene_node` itself is also checked.
 */
void Scene::replaceBlenderEntities(Ogre::SceneNode* p_scene_node)
{
    if (p_scene_node->numAttachedObjects() > 0) { // Transient nodes have zero entities
        assert(p_scene_node->numAttachedObjects() == 1); // blender2ogre always exports 1 node per entity
        Ogre::MovableObject* p_entity = p_scene_node->getAttachedObject(0);

        if (p_entity->getUserObjectBindings().getUserAny("il_entity").has_value()) {
            int etype = Ogre::any_cast<int>(p_entity->getUserObjectBindings().getUserAny("il_entity"));
            assert(etype > 0 && etype < static_cast<int>(entity_type::fin));

            cout << "il_entity type " << etype << " found! Replacing it." << endl;
            replaceBlenderEntity(p_scene_node, static_cast<entity_type>(etype));
            return;
        }
    }

    for(Ogre::Node* p_child: p_scene_node->getChildren()) {
        replaceBlenderEntities(static_cast<Ogre::SceneNode*>(p_child));
    }
}

void Scene::replaceBlenderEntity(Ogre::SceneNode* p_scene_node, entity_type etype)
{
    assert(p_scene_node->numAttachedObjects() == 1);
    Ogre::MovableObject* p_blender_entity = p_scene_node->detachObject(static_cast<short unsigned int>(0));
    Ogre::Entity* p_new_entity = nullptr;

    switch (etype) {
    case entity_type::sign:
        cout << "Replacing " << p_scene_node->getName() << " with a sign. " << endl;
        cout << "The sign should read '" << ExternalText::fetchExternalText(Ogre::any_cast<int>(p_blender_entity->getUserObjectBindings().getUserAny("il_text"))) << "'." << endl;
        p_new_entity = mp_scene_manager->createEntity("sign.mesh");
        break;
    default:
        throw(runtime_error(string("Invalid target entity type: ") + to_string(static_cast<int>(etype))));
    }

    p_scene_node->attachObject(p_new_entity);
}

/**
 * Called from the main loop when a keyboard key is pressed.
 * Please note that, regardless of whatever locale the user
 * has active, the `key` will always be the US keyboard key.
 * This is a restriction enforced by GLFW itself, see
 * <https://www.glfw.org/docs/3.3/group__keys.html>.
 *
 * This method does nothing by default.
 */
void Scene::processKeyInput(int key, int scancode, int action, int mods)
{
}
