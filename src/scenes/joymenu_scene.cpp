#include "joymenu_scene.hpp"
#include "../core/application.hpp"
#include "../core/window.hpp"
#include "../core/i18n.hpp"
#include "../core/timer.hpp"
#include "../core/game_state.hpp"
#include <OGRE/OgreTextureManager.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include <cstdio>

// Height of the images displayed for the control buttons, etc.
#define CTRL_WIDGET_HEIGHT 128.0f

using namespace std;
using namespace SceneSystem;
using namespace Core;

/// Calculate the ImGui cursor X start position so that an ImGui::Text
/// will come out exactly at the centre of the available horizontal space.
static void centreCursorForTextX(const char* text)
{
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize(text).x) / 2.0);
}

/** Calculate the ImGui cursor Y start position so that an ImGui::Text
 * will come out exactly at the centre of the vertical table space,
 * provided that the table cell is CTRL_WIDGET_HEIGHT pixels high.
 * The centering is calculated based on the font size of the active
 * Imgui font. This might be a little off the actual vertical centre,
 * but it should be sufficient. */
static void centreCursorForTextY()
{
    ImFont* p_font = ImGui::GetFont();
    assert(p_font);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (CTRL_WIDGET_HEIGHT - p_font->FontSize) / 2.0);
}

static void findChangedAxis(const int& len, const float neutral_joyaxes[], const float joyaxes[], int& axisno, float& limit)
{
    static float tolerance = 0.25f; // Reading from the joystick will not yield always the exact same values even if the sticks are left alone. Allow some tolerance.
    assert(len > 0);

    for(int i=0; i < len; i++) {
        printf("Axis %d has %.2f at neutral and %.2f now\n", i, neutral_joyaxes[i], joyaxes[i]);
        if (fabs(neutral_joyaxes[i] - joyaxes[i]) > tolerance) {
            printf(">>> Axis %d differs from neutral position\n", i);
            axisno = i;
            limit = joyaxes[i];
            return;
        }
    }
}

/// Formats a human-readable entry for the gamepad combo box UI for the GLFW joystick
/// with index `joyindex'.
static inline std::string joyStickComboItemName(int joyindex)
{
    return to_string(joyindex+1) + ": " + glfwGetJoystickName(joyindex);
}

JoymenuScene::JoymenuScene()
    : Scene("Joystick configuration menu scene"),
      mp_ui_system(nullptr),
      m_crossedcircle_tex(0),
      m_crossedcircle_yellow_tex(0),
      m_steercross_tex(0),
      m_buttons_tex(0),
      m_shoulderbuttons_tex(0),
      mp_config_timer(nullptr),
      m_joyconfig_stage(joyconfig_stage::none),
      mp_neutral_joyaxes(nullptr),
      m_config_item(configured_item::none)
{
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->addSceneManager(mp_scene_manager);

    // Enable the overlay system for this scene
    mp_scene_manager->addRenderQueueListener(Ogre::OverlaySystem::getSingletonPtr());

    // Enable UI
    mp_ui_system = new UISystem::GUIEngine();

    Ogre::SceneNode* p_cam_node = mp_scene_manager->getRootSceneNode()->createChildSceneNode();
    p_cam_node->setPosition(0, 0, 0);
    Ogre::Camera* p_camera = mp_scene_manager->createCamera("myCam");
    p_camera->setNearClipDistance(0.1);
    p_camera->setAutoAspectRatio(true);
    Application::getSingleton().getWindow().getOgreRenderWindow()->addViewport(p_camera);

    Ogre::TexturePtr ptr = Ogre::TextureManager::getSingleton().load("crossedcircle.png", "ui", Ogre::TEX_TYPE_2D, 1, 1.0f, Ogre::PF_BYTE_RGBA);
    assert(ptr);
    m_crossedcircle_tex = ptr->getHandle();

    ptr = Ogre::TextureManager::getSingleton().load("crossedcircle-yellow.png", "ui", Ogre::TEX_TYPE_2D, 1, 1.0f, Ogre::PF_BYTE_RGBA);
    assert(ptr);
    m_crossedcircle_yellow_tex = ptr->getHandle();

    ptr = Ogre::TextureManager::getSingleton().load("steeringcross.png", "ui", Ogre::TEX_TYPE_2D, 1, 1.0f, Ogre::PF_BYTE_RGBA);
    assert(ptr);
    m_steercross_tex = ptr->getHandle();

    ptr = Ogre::TextureManager::getSingleton().load("buttons.png", "ui", Ogre::TEX_TYPE_2D, 1, 1.0f, Ogre::PF_BYTE_RGBA);
    assert(ptr);
    m_buttons_tex = ptr->getHandle();

    ptr = Ogre::TextureManager::getSingleton().load("shoulderbuttons.png", "ui", Ogre::TEX_TYPE_2D, 1, 1.0f, Ogre::PF_BYTE_RGBA);
    assert(ptr);
    m_shoulderbuttons_tex = ptr->getHandle();
}

JoymenuScene::~JoymenuScene()
{
    if (mp_config_timer) {
        delete mp_config_timer;
        mp_config_timer = nullptr;
    }
    if (mp_neutral_joyaxes) {
        delete[] mp_neutral_joyaxes;
    }

    Application::getSingleton().getWindow().getOgreRenderWindow()->removeAllViewports();
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->removeSceneManager(mp_scene_manager);
    delete mp_ui_system;
}

void JoymenuScene::update()
{
    if (mp_config_timer) {
        mp_config_timer->update();
    }
    updateUI();
}

void JoymenuScene::updateUI()
{
    mp_ui_system->update();
    updateGamepadConfigUI(FREYA);
    updateGamepadConfigUI(BENJAMIN);

    switch (m_config_item) {
    case configured_item::none:
        break;
    case configured_item::control_stick:
    case configured_item::camera_stick: // fall-through
        updateJoystickConfig(FREYA);
        break;
    } // No default so the compiler warns about missed values
}

void JoymenuScene::updateGamepadConfigUI(int player)
{
    switch (player) {
    case FREYA:
        ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
        ImGui::SetNextWindowSize(ImVec2(620.0f, 700.0f));
        ImGui::Begin(_("Gamepad Configuration Player 1 (Freya)"), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        break;
    case BENJAMIN:
        ImGui::SetNextWindowPos(ImVec2(650.0f, 10.0f));
        ImGui::SetNextWindowSize(ImVec2(620.0f, 700.0f));
        ImGui::Begin(_("Gamepad Configuration Player 2 (Benjamin)"), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        break;
    default:
        assert(false);
    }

    auto& plyconf = GameState::instance.config[player];

    // Gamepad selection combobox
    int    active_gamepad = -1;
    string preview        = _("Select gamepad to use");
    if (glfwJoystickPresent(plyconf.joy_index)) {
        active_gamepad = plyconf.joy_index;
        preview        = joyStickComboItemName(active_gamepad);
    }
    if (ImGui::BeginCombo(_("Gamepad"), preview.c_str())) { // Returns true only if the element is opened
        for(int i=GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
            if (glfwJoystickPresent(i)) {
                if (ImGui::Selectable(joyStickComboItemName(i).c_str(), active_gamepad == i)) { // Returns true if clicked
                    active_gamepad = i;
                }
                if (active_gamepad == i) {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }
        ImGui::EndCombo();
    }

    // Do not render rest of UI until a gamepad is selected.
    if (active_gamepad == -1) {
        return;
    } else {
        assert(glfwJoystickPresent(active_gamepad)); // If this triggers, the player quickly removed the joystick after selecting it!
        plyconf.joy_index = active_gamepad;
    }

    // The large configuration table
    ImGui::BeginTable("table", 6, ImGuiTableFlags_SizingFixedFit);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    centreCursorForTextX(_("Steering"));
    ImGui::Text(_("Steering"));
    ImGui::TableSetColumnIndex(4);
    centreCursorForTextX(_("Camera"));
    ImGui::Text(_("Camera"));

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    string str = to_string(plyconf.joy_vertical.axisno) + (plyconf.joy_vertical.inverted ? "+" : "-");
    centreCursorForTextX(str.c_str());
    ImGui::Text(str.c_str());
    ImGui::TableSetColumnIndex(4);
    str = to_string(plyconf.joy_cam_vertical.axisno) + (plyconf.joy_cam_vertical.inverted ? "+" : "-");
    centreCursorForTextX(str.c_str());
    ImGui::Text(str.c_str());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    centreCursorForTextY();
    str = to_string(plyconf.joy_horizontal.axisno) + (plyconf.joy_horizontal.inverted ? "+" : "-");
    ImGui::Text(str.c_str());
    ImGui::TableSetColumnIndex(1);
    ImVec2 currpos = ImGui::GetCursorPos();
    ImGui::Dummy(ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    if (ImGui::IsItemHovered()) {
        ImGui::SetCursorPos(currpos);
        ImGui::Image(reinterpret_cast<ImTextureID>(m_crossedcircle_yellow_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    } else {
        ImGui::SetCursorPos(currpos);
        ImGui::Image(reinterpret_cast<ImTextureID>(m_crossedcircle_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        m_config_item     = configured_item::control_stick;
        m_joyconfig_stage = joyconfig_stage::neutral;
    }
    ImGui::TableSetColumnIndex(2);
    centreCursorForTextY();
    str = to_string(plyconf.joy_horizontal.axisno) + (plyconf.joy_horizontal.inverted ? "-" : "+");
    ImGui::Text(str.c_str());
    ImGui::TableSetColumnIndex(3);
    centreCursorForTextY();
    str = to_string(plyconf.joy_cam_horizontal.axisno) + (plyconf.joy_cam_horizontal.inverted ? "+" : "-");
    ImGui::Text(str.c_str());
    ImGui::TableSetColumnIndex(4);
    currpos = ImGui::GetCursorPos();
    ImGui::Dummy(ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    if (ImGui::IsItemHovered()) {
        ImGui::SetCursorPos(currpos);
        ImGui::Image(reinterpret_cast<ImTextureID>(m_crossedcircle_yellow_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    } else {
        ImGui::SetCursorPos(currpos);
        ImGui::Image(reinterpret_cast<ImTextureID>(m_crossedcircle_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        m_config_item     = configured_item::camera_stick;
        m_joyconfig_stage = joyconfig_stage::neutral;
    }
    ImGui::TableSetColumnIndex(5);
    centreCursorForTextY();
    str = to_string(plyconf.joy_cam_horizontal.axisno) + (plyconf.joy_cam_horizontal.inverted ? "-" : "+");
    ImGui::Text(str.c_str());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    str = to_string(plyconf.joy_vertical.axisno) + (plyconf.joy_vertical.inverted ? "-" : "+");
    centreCursorForTextX(str.c_str());
    ImGui::Text(str.c_str());
    ImGui::TableSetColumnIndex(4);
    str = to_string(plyconf.joy_cam_vertical.axisno) +  (plyconf.joy_cam_vertical.inverted ? "-" : "+");
    centreCursorForTextX(str.c_str());
    ImGui::Text(str.c_str());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    centreCursorForTextX(_("Items/Action"));
    ImGui::Text(_("Items/Action"));
    ImGui::TableSetColumnIndex(4);
    centreCursorForTextX(_("Spells"));
    ImGui::Text(_("Spells"));

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    centreCursorForTextX("6");
    ImGui::Text("6");
    ImGui::TableSetColumnIndex(4);
    centreCursorForTextX("1");
    ImGui::Text("1");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    centreCursorForTextY();
    ImGui::Text("7");
    ImGui::TableSetColumnIndex(1);
    ImGui::Image(reinterpret_cast<ImTextureID>(m_steercross_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    ImGui::TableSetColumnIndex(2);
    centreCursorForTextY();
    ImGui::Text("5");
    ImGui::TableSetColumnIndex(3);
    centreCursorForTextY();
    ImGui::Text("2");
    ImGui::TableSetColumnIndex(4);
    ImGui::Image(reinterpret_cast<ImTextureID>(m_buttons_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    ImGui::TableSetColumnIndex(5);
    centreCursorForTextY();
    ImGui::Text("3");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    centreCursorForTextX("1");
    ImGui::Text("1");
    ImGui::TableSetColumnIndex(4);
    centreCursorForTextX("4");
    ImGui::Text("4");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    centreCursorForTextX(_("Attack/Defence"));
    ImGui::Text(_("Attack/Defence"));
    ImGui::TableSetColumnIndex(4);
    // TRANS: Heading for miscellaneous controls in the joystick config menu
    centreCursorForTextX(_("Other"));
    ImGui::Text(_("Other"));

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    centreCursorForTextY();
    ImGui::Text("5");
    ImGui::TableSetColumnIndex(1);
    ImGui::Image(reinterpret_cast<ImTextureID>(m_shoulderbuttons_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    ImGui::TableSetColumnIndex(2);
    centreCursorForTextY();
    ImGui::Text("6");
    ImGui::TableSetColumnIndex(4);
    // TRANS: Label for the Menu button, keep the brackets
    ImGui::Text(_("[MENU]"));
    // TRANS: Label for the HUD (= Head Up Display) button, keep the brackets
    ImGui::Text(_("[HUD]"));
    ImGui::TableSetColumnIndex(5);
    ImGui::Text("9");
    ImGui::Text("8");
    ImGui::EndTable();

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Save Configuration").x - ImGui::GetStyle().FramePadding.x - 5);
    ImGui::Button(_("Save Configuration"));

    ImGui::End();
}

void JoymenuScene::updateJoystickConfig(int player)
{
    assert(m_config_item == configured_item::control_stick ||
           m_config_item == configured_item::camera_stick);

    // TODO: Honour "player"
    auto& plyconf = GameState::instance.config[FREYA];

    switch (m_joyconfig_stage) {
    case joyconfig_stage::none:
        break;
    case joyconfig_stage::neutral:
        ImGui::SetNextWindowPos(ImVec2(100.0f, 100.0f));
        ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
        ImGui::Begin(_("Configuring joystick"), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        ImGui::TextWrapped(_("Determining neutral positions. Please do not touch your joystick until the next prompt appears."));
        ImGui::NewLine();

        if (mp_config_timer) {
            centreCursorForTextX("0");
            ImGui::Text("%.0f", 5.0f - mp_config_timer->passedTime());
        } else {
            centreCursorForTextX(_("Start"));
            if (ImGui::Button(_("Start"))) {
                mp_config_timer = new Timer(5000.0, false, [&](){
                    // Clear memory from previous run, if any
                    if (mp_neutral_joyaxes) {
                        delete[] mp_neutral_joyaxes;
                    }

                    // Read neutral axes positions and remember them for later
                    int axescount = 0;
                    const float* joyaxes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axescount);
                    mp_neutral_joyaxes = new float[axescount];
                    memcpy(mp_neutral_joyaxes, joyaxes, sizeof(float) * axescount);

                    // Advance to next config stage
                    delete mp_config_timer;
                    mp_config_timer = nullptr;
                    m_joyconfig_stage = joyconfig_stage::vertical;
                });
            }
        }

        ImGui::End();

        break;
    case joyconfig_stage::vertical:
        ImGui::SetNextWindowPos(ImVec2(100.0f, 100.0f));
        ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
        ImGui::Begin(_("Configuring joystick"), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        switch (m_config_item) {
        case configured_item::control_stick:
            ImGui::TextWrapped(_("Determining vertical MOVEMENT axis. Please press UP until the next prompt appears."));
            break;
        case configured_item::camera_stick:
            ImGui::TextWrapped(_("Determining vertical CAMERA STEERING axis. Please press UP until the next prompt appears."));
            break;
        default:
            assert(false);
            break;
        }
        ImGui::NewLine();

        if (mp_config_timer) {
            centreCursorForTextX("0");
            ImGui::Text("%.0f", 5.0f - mp_config_timer->passedTime());
        } else {
            centreCursorForTextX(_("Start"));
            if (ImGui::Button(_("Start"))) {
                mp_config_timer = new Timer(5000.0, false, [&](){
                    // The changed axis is the vertical axis
                    int axis = 0;
                    int axescount = 0;
                    float axislimit = 0.0f;
                    const float* joyaxes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axescount);
                    findChangedAxis(axescount, mp_neutral_joyaxes, joyaxes, axis, axislimit);

                    if (m_config_item == configured_item::control_stick) {
                        plyconf.joy_vertical.axisno = axis;
                        plyconf.joy_vertical.inverted = axislimit > 0.0f;
                    } else {
                        plyconf.joy_cam_vertical.axisno = axis;
                        plyconf.joy_cam_vertical.inverted = axislimit > 0.0f;
                    }

                    // Advance to next config stage
                    delete mp_config_timer;
                    mp_config_timer = nullptr;
                    m_joyconfig_stage = joyconfig_stage::horizontal;
                });
            }
        }

        ImGui::End();
        break;
    case joyconfig_stage::horizontal:
        ImGui::SetNextWindowPos(ImVec2(100.0f, 100.0f));
        ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
        ImGui::Begin(_("Configuring joystick"), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
        switch(m_config_item) {
        case configured_item::control_stick:
            ImGui::TextWrapped(_("Determining horizontal MOVEMENT axis. Please press RIGHT until this prompt disappears."));
            break;
        case configured_item::camera_stick:
            ImGui::TextWrapped(_("Determining horizontal CAMERA STEERING axis. Please press RIGHT until this prompt disappears."));
            break;
        default:
            assert(false);
        }
        ImGui::NewLine();

        if (mp_config_timer) {
            centreCursorForTextX("0");
            ImGui::Text("%.0f", 5.0f - mp_config_timer->passedTime());
        } else {
            centreCursorForTextX(_("Start"));
            if (ImGui::Button(_("Start"))) {
                mp_config_timer = new Timer(5000.0, false, [&](){
                    // The changed axis is the horizontal axis
                    int axis = 0;
                    int axescount = 0;
                    float axislimit = 0.0f;
                    const float* joyaxes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axescount);
                    findChangedAxis(axescount, mp_neutral_joyaxes, joyaxes, axis, axislimit);

                    if (m_config_item == configured_item::control_stick) {
                        plyconf.joy_horizontal.axisno = axis;
                        plyconf.joy_horizontal.inverted = axislimit < 0.0f;
                    } else {
                        plyconf.joy_cam_horizontal.axisno = axis;
                        plyconf.joy_cam_horizontal.inverted = axislimit < 0.0f;
                    }

                    /* Calculate the dead zone. In order to have both control sticks
                     * behave equally, the dead zone is calculated for the more noisy
                     * one and applied to the other one, i.e. both are set up with
                     * the same dead zone. Don't just get the maximum value of all
                     * of mp_neutral_joyaxes, because some joysticks have more axes
                     * than the control sticks, e.g. in the shoulder buttons. Also,
                     * 0.1 is added to cater for possible larger noise. */
                    plyconf.joy_dead_zone = max({mp_neutral_joyaxes[plyconf.joy_horizontal.axisno],
                                                 mp_neutral_joyaxes[plyconf.joy_vertical.axisno],
                                                 mp_neutral_joyaxes[plyconf.joy_cam_horizontal.axisno],
                                                 mp_neutral_joyaxes[plyconf.joy_cam_vertical.axisno]})
                                            + 0.1f;
                    printf("Dead zone calculated as %.2f\n", plyconf.joy_dead_zone);

                    // End config stages
                    delete mp_config_timer;
                    mp_config_timer = nullptr;
                    m_joyconfig_stage = joyconfig_stage::none;
                    m_config_item = configured_item::none;
                });
            }
        }

        ImGui::End();
        break;
    } // No default to have the compiler warn on missing items
}

void JoymenuScene::processKeyInput(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE) {
        finish();
    } else {
        mp_ui_system->processKeyInput(key, scancode, action, mods);
    }
}

void JoymenuScene::processCharInput(unsigned int codepoint)
{
    mp_ui_system->processCharInput(codepoint);
}

void JoymenuScene::processMouseButton(int button, int action, int mods)
{
    mp_ui_system->processMouseButton(button, action, mods);
}

void JoymenuScene::processCursorMove(double xpos, double ypos)
{
    mp_ui_system->processCursorMove(xpos, ypos);
}
