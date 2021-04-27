#ifndef RPG_DUMMY_SCENE_HPP
#define RPG_DUMMY_SCENE_HPP
#include "scene.hpp"

namespace Ogre {
    class SceneNode;
}

namespace SceneSystem {

    class DummyScene: public Scene
    {
    public:
        DummyScene();
        virtual ~DummyScene();

        virtual void update();
    private:
        Ogre::SceneNode* mp_cube_node;
    };

}

#endif /* RPG_DUMMY_SCENE_HPP */
