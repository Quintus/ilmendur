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
        virtual void activate();
        virtual void deactivate();
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
        Ogre::ResourceHandle m_steercross_yellow_tex;
        Ogre::ResourceHandle m_buttons_tex;
        Ogre::ResourceHandle m_buttons_yellow_tex;
        Ogre::ResourceHandle m_shoulderbuttons_tex;
        Ogre::ResourceHandle m_shoulderbuttons_yellow_tex;
        Ogre::Camera* mp_camera;

        Core::Timer* mp_config_timer;
        std::vector<float> m_neutral_joyaxes[2];

        enum class joyconfig_stage {
            none = 0,
            vertical,
            horizontal
        };
        joyconfig_stage m_joyconfig_stage;

        enum class hatchconfig_stage {
            none = 0,
            up,
            right,
            down,
            left
        };
        hatchconfig_stage m_hatchconfig_stage;

        enum class actbuttonconfig_stage {
            none = 0,
            top,
            right,
            bottom,
            left
        };
        actbuttonconfig_stage m_actbuttonconfig_stage;

        enum class shoulderbuttonconfig_stage {
            none = 0,
            left,
            right
        };
        shoulderbuttonconfig_stage m_shoulderbuttonconfig_stage;

        enum class menubuttonconfig_stage {
            none = 0,
            menu,
            hud
        };
        menubuttonconfig_stage m_menubuttonconfig_stage;

        enum class configured_item {
            none = 0,
            control_stick,
            camera_stick,
            hatch,
            action_buttons,
            shoulder_buttons,
            menu_buttons
        };
        configured_item m_config_item;
        int m_config_player;
        float m_tabstops[4];

        void updateUI();
        void updateGamepadConfigUI();
        bool updateGamepadConfig_Gamepad(int player);
        void updateGamepadConfigTable_JoysticksTitles(int player);
        void updateGamepadConfigTable_JoysticksTopLabels(int player);
        void updateGamepadConfigTable_JoysticksMainRow(int player);
        void updateGamepadConfigTable_JoysticksBottomLabels(int player);
        void updateGamepadConfigTable_ItemsActionsTitles(int player);
        void updateGamepadConfigTable_ItemsActionsTopLabels(int player);
        void updateGamepadConfigTable_ItemsActionsMainRow(int player);
        void updateGamepadConfigTable_ItemsActionsBottomLabels(int player);
        void updateGamepadConfigTable_AttackDefenceOtherTitles(int player);
        void updateGamepadConfigTable_AttackDefenceOtherMainRow(int player);
        void readNeutralPositions(int player);
        void updateJoystickConfig(int player);
        void updateHatchConfig(int player);
        void updateActionButtonConfig(int player);
        void updateShoulderButtonConfig(int player);
        void updateMenuButtonConfig(int player);
    };

}

#endif /* ILMENDUR_JOYMENU_SCENE_HPP */
