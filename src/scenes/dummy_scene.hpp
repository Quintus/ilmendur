#ifndef ILMENDUR_DUMMY_SCENE_HPP
#define ILMENDUR_DUMMY_SCENE_HPP
#include "scene.hpp"
#include "../physics/physics.hpp"

class StaticGeometry;
class Freya;

namespace SceneSystem {

    class DummyScene: public Scene
    {
    public:
        DummyScene();
        virtual ~DummyScene();
        virtual void activate();
        virtual void deactivate();

        virtual void processKeyInput(int key, int scancode, int action, int mods);
        virtual void processCharInput(unsigned int codepoint);
        virtual void processMouseButton(int button, int action, int mods);
        virtual void processCursorMove(double xpos, double ypos);
        virtual void update();
        virtual void draw();
    private:
        void calculateJoyZones();
        void handleJoyInput();
        void handleCamJoyInput();
        void handleMoveJoyInput();

        Ogre::SceneNode* mp_area_node;
        Ogre::SceneNode* mp_camera_target;
        Ogre::SceneNode* mp_cam_node;
        Ogre::Camera* mp_camera;
        StaticGeometry* mp_ground;
        Freya* mp_player;

        float m_run_threshold;
    };

}

#endif /* ILMENDUR_DUMMY_SCENE_HPP */
