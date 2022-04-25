#ifndef ILMENDUR_JOYMENU_SCENE_HPP
#define ILMENDUR_JOYMENU_SCENE_HPP
#include "scene.hpp"
#include "../ui/ui.hpp"

namespace Core {
    class Timer;
}

namespace SceneSystem {
    class JoymenuScene: public Scene
    {
    public:
        JoymenuScene();
        virtual ~JoymenuScene();
        virtual void processKeyInput(int key, int scancode, int action, int mods);
        virtual void processCharInput(unsigned int codepoint);
        virtual void processMouseButton(int button, int action, int mods);
        virtual void processCursorMove(double xpos, double ypos);
        virtual void update();
    private:
        UISystem::GUIEngine* mp_ui_system;
        Ogre::ResourceHandle m_crossedcircle_tex;
        Ogre::ResourceHandle m_crossedcircle_yellow_tex;
        Ogre::ResourceHandle m_steercross_tex;
        Ogre::ResourceHandle m_buttons_tex;
        Ogre::ResourceHandle m_shoulderbuttons_tex;

        Core::Timer* mp_config_timer;

        enum class joyconfig_stage {
            none = 0,
            vertical,
            horizontal,
            dead_zone
        };
        joyconfig_stage m_joyconfig_stage;
    };

}

#endif /* ILMENDUR_JOYMENU_SCENE_HPP */
