#ifndef ILMENDUR_DUMMY_SCENE_HPP
#define ILMENDUR_DUMMY_SCENE_HPP
#include "scene.hpp"

namespace Ogre {
    class SceneNode;
    class ManualObject;
}

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
    };

}

#endif /* ILMENDUR_DUMMY_SCENE_HPP */
