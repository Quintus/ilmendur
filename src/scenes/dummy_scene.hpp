#ifndef ILMENDUR_DUMMY_SCENE_HPP
#define ILMENDUR_DUMMY_SCENE_HPP
#include "scene.hpp"

namespace Ogre {
    class SceneNode;
    class ManualObject;
}

class btRigidBody;
class btDiscreteDynamicsWorld;

namespace SceneSystem {

    class DummyScene: public Scene
    {
    public:
        DummyScene();
        virtual ~DummyScene();

        virtual void processKeyInput(int key, int scancode, int action, int mods);
        virtual void update();
    private:
        Ogre::SceneNode* mp_area_node;
        Ogre::SceneNode* mp_cam_node;
        Ogre::SceneNode* mp_player_node;
        btRigidBody* mp_player_rbody;
        btDiscreteDynamicsWorld* mp_bullet_world;
    };

}

#endif /* ILMENDUR_DUMMY_SCENE_HPP */
