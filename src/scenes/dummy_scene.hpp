#ifndef ILMENDUR_DUMMY_SCENE_HPP
#define ILMENDUR_DUMMY_SCENE_HPP
#include "scene.hpp"
#include "../physics/physics.hpp"

class Freya;

namespace SceneSystem {

    class DummyScene: public Scene
    {
    public:
        DummyScene();
        virtual ~DummyScene();

        virtual void processKeyInput(int key, int scancode, int action, int mods);
        virtual void update();
    private:
        PhysicsSystem::PhysicsEngine m_physics;
        Ogre::SceneNode* mp_area_node;
        Ogre::SceneNode* mp_cam_node;
        Freya* mp_player;
    };

}

#endif /* ILMENDUR_DUMMY_SCENE_HPP */
