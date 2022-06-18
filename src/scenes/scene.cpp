#include "scene.hpp"
#include "../entities/entity.hpp"
#include "../exttexts/exttexts.hpp"
#include <cassert>
#include <iostream>

using namespace SceneSystem;
using namespace std;

/**
 * Constructor. Override in subclasses to your likening.
 * However, DO NOT acquire global resources like Ogre's render
 * window here, because your scene might be constructed in the
 * background while another scene is running. Override activate()
 * and deactivate() to deal with the acquisisation of global resources.
 */
Scene::Scene(const string& name)
    : mp_physics(nullptr),
      mp_scene_manager(Ogre::Root::getSingleton().createSceneManager()),
      m_name(name),
      m_finish(false),
      mp_next_scene(nullptr)
{
}

/**
 * Destructor. Override in subclasses to your likening. However,
 * DO NOT release global resources like Ogre's render window
 * here, because it might not even be owned by your scene
 * anymore at the point the destructor runs. Use deactivate()
 * for releasing such resources.
 */
Scene::~Scene()
{
    Ogre::Root::getSingleton().destroySceneManager(mp_scene_manager);
    mp_scene_manager = nullptr;
}

/**
 * Called when this Scene instance becomes the top of the scene
 * stack. It is guaranteed to be called after the constructor
 * has completed.
 *
 * Use this method to acquire global resources like Ogre's render window,
 * and release said resources in deactivate().
 */
void Scene::activate()
{
}

/**
 * Called when this Scene gets pushed down or deleted from the scene
 * stack. Guaranteed to be called before the destructor runs.
 *
 * Use this method to release the global resources acquired by
 * activate().
 */
void Scene::deactivate()
{
}

/**
 * Set the Finishing flag on this scene. The scene stack controller
 * code will pop the scene from the stack if after executing the
 * current frame this mark is set. It can be queried with isFinishing().
 *
 * \param[in] p_next
 * The scene to transition to. This is what the scene stack controller
 * code will put onto the scene stack after it popped the finishing scene.
 * Do not use the pointer anymore after it was passed here; the scene
 * stack controller assumes control of it. If this is NULL, nothing
 * will be pushed onto the scene stack.
 *
 * \remark Using `p_next' is preferred to using Application::pushScene()
 * if it is your intention not to return to this scene.
 * See there for the reasons.
 */
void Scene::finish(Scene* p_next)
{
    assert(!isFinishing());

    m_finish = true;
    mp_next_scene = p_next;
}

/**
 * Whether the scene is about to finish, see finish().
 */
bool Scene::isFinishing()
{
    return m_finish;
}

/**
 * \private Access and clear the next scene. Private API to be
 * only used in Application::run().
 */
Scene* Scene::nextScene()
{
    assert(isFinishing());

    Scene* p_scene = mp_next_scene;
    mp_next_scene = nullptr;
    return p_scene;
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

/**
 * Called from the main loop when Unicode chars are input.
 * Does nothing by default.
 */
void Scene::processCharInput(unsigned int codepoint)
{
}

/**
 * Called from the main loop when the mouse cursor is moved.
 * Receives the new position. Does nothing by default.
 */
void Scene::processCursorMove(double xpos, double ypos)
{
}

/**
 * Called from the main loop when the mouse is clicked.
 * Arguments are the same as with GLFW's mouse button input
 * callback. Does nothing by default.
 */
void Scene::processMouseButton(int button, int action, int mods)
{
}
