#include "joymenu_scene.hpp"
#include "dummy_scene.hpp"
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
        if (fabs(neutral_joyaxes[i] - joyaxes[i]) > tolerance) {
            axisno = i;
            limit = joyaxes[i];
            return;
        }
    }

    // FIXME: If the user presses nothing, this is reached
    assert(false);
}

/// Queries all buttons on the joystick `joy_index' and returns the
/// first one that is pressed; returns -1 if no button is pressed.
static int findPressedButton(int joy_index)
{
    int button_count             = 0;
    const unsigned char* buttons = glfwGetJoystickButtons(joy_index, &button_count);

    for(int i=0; i < button_count; i++) {
        if (buttons[i] == GLFW_PRESS) {
            return i;
        }
    }

    return -1;
}

/// Blocks until the button `button' on the joystick `joy_index' is released.
static void waitUntilButtonReleased(int joy_index, int button)
{
    int button_count             = 0;
    const unsigned char* buttons = glfwGetJoystickButtons(joy_index, &button_count);

    assert(button < button_count);
    while (buttons[button] == GLFW_PRESS) {
        buttons = glfwGetJoystickButtons(joy_index, &button_count);
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
      m_steercross_yellow_tex(0),
      m_buttons_tex(0),
      m_buttons_yellow_tex(0),
      m_shoulderbuttons_tex(0),
      m_shoulderbuttons_yellow_tex(0),
      mp_config_timer(nullptr),
      m_joyconfig_stage(joyconfig_stage::none),
      m_hatchconfig_stage(hatchconfig_stage::none),
      m_actbuttonconfig_stage(actbuttonconfig_stage::none),
      m_shoulderbuttonconfig_stage(shoulderbuttonconfig_stage::none),
      m_menubuttonconfig_stage(menubuttonconfig_stage::none),
      m_config_item(configured_item::none),
      m_config_player(-1)
{
    m_tabstops[0] = 0.0f;
    m_tabstops[1] = 0.0f;
    m_tabstops[2] = 0.0f;
    m_tabstops[3] = 0.0f;

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

    ptr = Ogre::TextureManager::getSingleton().load("steeringcross-yellow.png", "ui", Ogre::TEX_TYPE_2D, 1, 1.0f, Ogre::PF_BYTE_RGBA);
    assert(ptr);
    m_steercross_yellow_tex = ptr->getHandle();

    ptr = Ogre::TextureManager::getSingleton().load("buttons.png", "ui", Ogre::TEX_TYPE_2D, 1, 1.0f, Ogre::PF_BYTE_RGBA);
    assert(ptr);
    m_buttons_tex = ptr->getHandle();

    ptr = Ogre::TextureManager::getSingleton().load("buttons-yellow.png", "ui", Ogre::TEX_TYPE_2D, 1, 1.0f, Ogre::PF_BYTE_RGBA);
    assert(ptr);
    m_buttons_yellow_tex = ptr->getHandle();

    ptr = Ogre::TextureManager::getSingleton().load("shoulderbuttons.png", "ui", Ogre::TEX_TYPE_2D, 1, 1.0f, Ogre::PF_BYTE_RGBA);
    assert(ptr);
    m_shoulderbuttons_tex = ptr->getHandle();

    ptr = Ogre::TextureManager::getSingleton().load("shoulderbuttons-yellow.png", "ui", Ogre::TEX_TYPE_2D, 1, 1.0f, Ogre::PF_BYTE_RGBA);
    assert(ptr);
    m_shoulderbuttons_yellow_tex = ptr->getHandle();

    readNeutralPositions(PLAYER1);
    readNeutralPositions(PLAYER2);
}

JoymenuScene::~JoymenuScene()
{
    if (mp_config_timer) {
        delete mp_config_timer;
        mp_config_timer = nullptr;
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
    updateGamepadConfigUI();

    switch (m_config_item) {
    case configured_item::none:
        break;
    case configured_item::control_stick:
    case configured_item::camera_stick: // fall-through
        updateJoystickConfig(m_config_player);
        break;
    case configured_item::hatch:
        updateHatchConfig(m_config_player);
        break;
    case configured_item::action_buttons:
        updateActionButtonConfig(m_config_player);
        break;
    case configured_item::shoulder_buttons:
        updateShoulderButtonConfig(m_config_player);
        break;
    case configured_item::menu_buttons:
        updateMenuButtonConfig(m_config_player);
        break;
    } // No default so the compiler warns about missed values
}

void JoymenuScene::updateGamepadConfigUI()
{
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
    ImGui::SetNextWindowSize(ImVec2(1260.0f, 700.0f));
    ImGui::Begin(_("Gamepad Configuration"), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    // Gamepad combo box
    ImGui::SetCursorPosX(m_tabstops[0]);
    ImGui::SetNextItemWidth(m_tabstops[1] - m_tabstops[0]);
    bool has_gamepad1 = updateGamepadConfig_Gamepad(PLAYER1);
    ImGui::SameLine(m_tabstops[2]);
    ImGui::SetNextItemWidth(m_tabstops[3] - m_tabstops[2]);
    bool has_gamepad2 = updateGamepadConfig_Gamepad(PLAYER2);

    // The large configuration table
    ImGui::BeginTable("table", 15, ImGuiTableFlags_SizingFixedFit);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch); // Spacer column
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch); // Divider column
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthStretch); // Spacer column

    // Joystick axes configurator
    ImGui::TableNextRow();
    if (has_gamepad1) { updateGamepadConfigTable_JoysticksTitles(PLAYER1); }
    if (has_gamepad2) { updateGamepadConfigTable_JoysticksTitles(PLAYER2); }

    ImGui::TableNextRow();
    if (has_gamepad1) { updateGamepadConfigTable_JoysticksTopLabels(PLAYER1); }
    if (has_gamepad2) { updateGamepadConfigTable_JoysticksTopLabels(PLAYER2); }

    ImGui::TableNextRow();
    if (has_gamepad1) { updateGamepadConfigTable_JoysticksMainRow(PLAYER1); }
    if (has_gamepad2) { updateGamepadConfigTable_JoysticksMainRow(PLAYER2); }

    ImGui::TableNextRow();
    if (has_gamepad1) { updateGamepadConfigTable_JoysticksBottomLabels(PLAYER1); }
    if (has_gamepad2) { updateGamepadConfigTable_JoysticksBottomLabels(PLAYER2); }

    // Items/Actions configurator
    ImGui::TableNextRow();
    if (has_gamepad1) { updateGamepadConfigTable_ItemsActionsTopLabels(PLAYER1); }
    if (has_gamepad2) { updateGamepadConfigTable_ItemsActionsTopLabels(PLAYER2); }

    ImGui::TableNextRow();
    if (has_gamepad1) { updateGamepadConfigTable_ItemsActionsMainRow(PLAYER1); }
    if (has_gamepad2) { updateGamepadConfigTable_ItemsActionsMainRow(PLAYER2); }

    ImGui::TableNextRow();
    if (has_gamepad1) { updateGamepadConfigTable_ItemsActionsBottomLabels(PLAYER1); }
    if (has_gamepad2) { updateGamepadConfigTable_ItemsActionsBottomLabels(PLAYER2); }

    // Attack/Defence/Other configurator
    ImGui::TableNextRow();
    if (has_gamepad1) { updateGamepadConfigTable_AttackDefenceOtherTitles(PLAYER1); }
    if (has_gamepad2) { updateGamepadConfigTable_AttackDefenceOtherTitles(PLAYER2); }

    ImGui::TableNextRow();
    if (has_gamepad1) { updateGamepadConfigTable_AttackDefenceOtherMainRow(PLAYER1); }
    if (has_gamepad2) { updateGamepadConfigTable_AttackDefenceOtherMainRow(PLAYER2); }
    ImGui::EndTable();

    // Confirmation button
    ImGui::SetCursorPosX(m_tabstops[3] - ImGui::CalcTextSize("Save Configuration").x);
    if (ImGui::Button(_("Save Configuration"))) {
        finish(new SceneSystem::DummyScene());
    }

    ImGui::End();
}

bool JoymenuScene::updateGamepadConfig_Gamepad(int player)
{
    auto& plyconf = GameState::instance.config[player];

    // Gamepad selection combobox
    string preview = _("Select gamepad to use");
    if (glfwJoystickPresent(plyconf.joy_index)) {
        preview = joyStickComboItemName(plyconf.joy_index);
    }

    // Ensure the combo boxes have different labels, otherwise Imgui
    // will mess them up.
    char label[256];
    snprintf(label, 256, _("Player %d"), player + 1);

    if (ImGui::BeginCombo(label, preview.c_str())) { // Returns true only if the element is opened
        for(int i=GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
            if (glfwJoystickPresent(i)) {
                if (ImGui::Selectable(joyStickComboItemName(i).c_str(), plyconf.joy_index == i)) { // Returns true if clicked
                    plyconf.joy_index = i;
                    readNeutralPositions(player);
                }
                if (plyconf.joy_index == i) {
                    ImGui::SetItemDefaultFocus();
                }
            }
        }
        ImGui::EndCombo();
    }

    return glfwJoystickPresent(plyconf.joy_index);
}

void JoymenuScene::updateGamepadConfigTable_JoysticksTitles(int player)
{
    int offset    = player == PLAYER1 ? 1 : 8;

    ImGui::TableSetColumnIndex(offset+1);
    centreCursorForTextX(_("Steering"));
    ImGui::Text(_("Steering"));
    ImGui::TableSetColumnIndex(offset+4);
    centreCursorForTextX(_("Camera"));
    ImGui::Text(_("Camera"));

    // Add dummy for spacing the two table sections
    if (player == PLAYER1) {
        ImGui::TableSetColumnIndex(6);
        ImGui::Dummy(ImVec2(CTRL_WIDGET_HEIGHT, 1));
    }
}

void JoymenuScene::updateGamepadConfigTable_JoysticksTopLabels(int player)
{
    int offset    = player == PLAYER1 ? 1 : 8;
    auto& plyconf = GameState::instance.config[player];

    ImGui::TableSetColumnIndex(offset+1);
    string str = to_string(plyconf.joy_vertical.axisno) + (plyconf.joy_vertical.inverted ? "+" : "-");
    centreCursorForTextX(str.c_str());
    ImGui::Text(str.c_str());
    ImGui::TableSetColumnIndex(offset+4);
    str = to_string(plyconf.joy_cam_vertical.axisno) + (plyconf.joy_cam_vertical.inverted ? "+" : "-");
    centreCursorForTextX(str.c_str());
    ImGui::Text(str.c_str());
}

void JoymenuScene::updateGamepadConfigTable_JoysticksMainRow(int player)
{
    int offset    = player == PLAYER1 ? 1 : 8;
    auto& plyconf = GameState::instance.config[player];

    ImGui::TableSetColumnIndex(offset+0);
    if (player == PLAYER1) {
        m_tabstops[0] = ImGui::GetCursorPosX();
    } else {
        m_tabstops[2] = ImGui::GetCursorPosX();
    }
    centreCursorForTextY();
    string str = to_string(plyconf.joy_horizontal.axisno) + (plyconf.joy_horizontal.inverted ? "+" : "-");
    ImGui::Text(str.c_str());
    ImGui::TableSetColumnIndex(offset+1);
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
        m_config_player   = player;
        m_config_item     = configured_item::control_stick;
        m_joyconfig_stage = joyconfig_stage::vertical;
    }
    ImGui::TableSetColumnIndex(offset+2);
    centreCursorForTextY();
    str = to_string(plyconf.joy_horizontal.axisno) + (plyconf.joy_horizontal.inverted ? "-" : "+");
    ImGui::Text(str.c_str());
    ImGui::TableSetColumnIndex(offset+3);
    centreCursorForTextY();
    str = to_string(plyconf.joy_cam_horizontal.axisno) + (plyconf.joy_cam_horizontal.inverted ? "+" : "-");
    ImGui::Text(str.c_str());
    ImGui::TableSetColumnIndex(offset+4);
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
        m_config_player   = player;
        m_config_item     = configured_item::camera_stick;
        m_joyconfig_stage = joyconfig_stage::vertical;
    }
    ImGui::TableSetColumnIndex(offset+5);
    centreCursorForTextY();
    str = to_string(plyconf.joy_cam_horizontal.axisno) + (plyconf.joy_cam_horizontal.inverted ? "-" : "+");
    ImGui::Text(str.c_str());

    if (player == PLAYER1) {
        m_tabstops[1] = ImGui::GetCursorPosX();
    } else {
        m_tabstops[3] = ImGui::GetCursorPosX();
    }
}

void JoymenuScene::updateGamepadConfigTable_JoysticksBottomLabels(int player)
{
    int offset    = player == PLAYER1 ? 1 : 8;
    auto& plyconf = GameState::instance.config[player];

    ImGui::TableSetColumnIndex(offset+1);
    string str = to_string(plyconf.joy_vertical.axisno) + (plyconf.joy_vertical.inverted ? "-" : "+");
    centreCursorForTextX(str.c_str());
    ImGui::Text(str.c_str());
    ImGui::TableSetColumnIndex(offset+4);
    str = to_string(plyconf.joy_cam_vertical.axisno) +  (plyconf.joy_cam_vertical.inverted ? "-" : "+");
    centreCursorForTextX(str.c_str());
    ImGui::Text(str.c_str());
}

void JoymenuScene::updateGamepadConfigTable_ItemsActionsTitles(int player)
{
    int offset    = player == PLAYER1 ? 1 : 8;

    ImGui::TableSetColumnIndex(offset+1);
    centreCursorForTextX(_("Items/Action"));
    ImGui::Text(_("Items/Action"));
    ImGui::TableSetColumnIndex(offset+4);
    centreCursorForTextX(_("Spells"));
    ImGui::Text(_("Spells"));
}

void JoymenuScene::updateGamepadConfigTable_ItemsActionsTopLabels(int player)
{
    int offset    = player == PLAYER1 ? 1 : 8;
    auto& plyconf = GameState::instance.config[player];

    ImGui::TableSetColumnIndex(offset+1);
    string str = to_string(plyconf.hatch_up);
    centreCursorForTextX(str.c_str());
    ImGui::Text(str.c_str());
    ImGui::TableSetColumnIndex(offset+4);
    str = to_string(plyconf.actbutton_up);
    centreCursorForTextX(str.c_str());
    ImGui::Text(str.c_str());
}

void JoymenuScene::updateGamepadConfigTable_ItemsActionsMainRow(int player)
{
    int offset    = player == PLAYER1 ? 1 : 8;
    auto& plyconf = GameState::instance.config[player];

    ImGui::TableSetColumnIndex(offset+0);
    centreCursorForTextY();
    ImGui::Text(to_string(plyconf.hatch_left).c_str());
    ImGui::TableSetColumnIndex(offset+1);
    ImVec2 currpos = ImGui::GetCursorPos();
    ImGui::Dummy(ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    if (ImGui::IsItemHovered()) {
        ImGui::SetCursorPos(currpos);
        ImGui::Image(reinterpret_cast<ImTextureID>(m_steercross_yellow_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    } else {
        ImGui::SetCursorPos(currpos);
        ImGui::Image(reinterpret_cast<ImTextureID>(m_steercross_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        m_config_player = player;
        m_config_item = configured_item::hatch;
        m_hatchconfig_stage = hatchconfig_stage::up;
    }
    ImGui::TableSetColumnIndex(offset+2);
    centreCursorForTextY();
    ImGui::Text(to_string(plyconf.hatch_right).c_str());
    ImGui::TableSetColumnIndex(offset+3);
    centreCursorForTextY();
    ImGui::Text(to_string(plyconf.actbutton_left).c_str());
    ImGui::TableSetColumnIndex(offset+4);
    currpos = ImGui::GetCursorPos();
    ImGui::Dummy(ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    if (ImGui::IsItemHovered()) {
        ImGui::SetCursorPos(currpos);
        ImGui::Image(reinterpret_cast<ImTextureID>(m_buttons_yellow_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    } else {
        ImGui::SetCursorPos(currpos);
        ImGui::Image(reinterpret_cast<ImTextureID>(m_buttons_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        m_config_player = player;
        m_config_item = configured_item::action_buttons;
        m_actbuttonconfig_stage = actbuttonconfig_stage::top;
    }
    ImGui::TableSetColumnIndex(offset+5);
    centreCursorForTextY();
    ImGui::Text(to_string(plyconf.actbutton_right).c_str());
}

void JoymenuScene::updateGamepadConfigTable_ItemsActionsBottomLabels(int player)
{
    int offset    = player == PLAYER1 ? 1 : 8;
    auto& plyconf = GameState::instance.config[player];

    ImGui::TableSetColumnIndex(offset+1);
    string str = to_string(plyconf.hatch_down);
    centreCursorForTextX(str.c_str());
    ImGui::Text(str.c_str());
    ImGui::TableSetColumnIndex(offset+4);
    str = to_string(plyconf.actbutton_down);
    centreCursorForTextX(str.c_str());
    ImGui::Text(str.c_str());
}

void JoymenuScene::updateGamepadConfigTable_AttackDefenceOtherTitles(int player)
{
    int offset    = player == PLAYER1 ? 1 : 8;

    ImGui::TableSetColumnIndex(offset+1);
    centreCursorForTextX(_("Attack/Defence"));
    ImGui::Text(_("Attack/Defence"));
    ImGui::TableSetColumnIndex(offset+4);
    // TRANS: Heading for miscellaneous controls in the joystick config menu
    centreCursorForTextX(_("Other"));
    ImGui::Text(_("Other"));
}

void JoymenuScene::updateGamepadConfigTable_AttackDefenceOtherMainRow(int player)
{
    int offset    = player == PLAYER1 ? 1 : 8;
    auto& plyconf = GameState::instance.config[player];

    ImGui::TableSetColumnIndex(offset+0);
    centreCursorForTextY();
    ImGui::Text(to_string(plyconf.shoulderbutton_left).c_str());
    ImGui::TableSetColumnIndex(offset+1);
    ImVec2 currpos = ImGui::GetCursorPos();
    ImGui::Dummy(ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    if (ImGui::IsItemHovered()) {
        ImGui::SetCursorPos(currpos);
        ImGui::Image(reinterpret_cast<ImTextureID>(m_shoulderbuttons_yellow_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    } else {
        ImGui::SetCursorPos(currpos);
        ImGui::Image(reinterpret_cast<ImTextureID>(m_shoulderbuttons_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        m_config_player = player;
        m_config_item = configured_item::shoulder_buttons;
        m_shoulderbuttonconfig_stage = shoulderbuttonconfig_stage::left;
    }
    ImGui::TableSetColumnIndex(offset+2);
    centreCursorForTextY();
    ImGui::Text(to_string(plyconf.shoulderbutton_right).c_str());
    ImGui::TableSetColumnIndex(offset+4);
    currpos = ImGui::GetCursorPos();
    ImGui::Dummy(ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    if (ImGui::IsItemHovered()) {
        ImGui::SetCursorPos(currpos);
        // TRANS: Label for the Menu button, keep the brackets
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), _("[MENU]"));
        // TRANS: Label for the HUD (= Head Up Display) button, keep the brackets
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), _("[HUD]"));
    } else {
        ImGui::SetCursorPos(currpos);
        ImGui::Text(_("[MENU]"));
        ImGui::Text(_("[HUD]"));
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
        m_config_player = player;
        m_config_item = configured_item::menu_buttons;
        m_menubuttonconfig_stage = menubuttonconfig_stage::menu;
    }
    ImGui::TableSetColumnIndex(offset+5);
    ImGui::Text(to_string(plyconf.menu_button).c_str());
    ImGui::Text(to_string(plyconf.hud_button).c_str());
}

void JoymenuScene::updateJoystickConfig(int player)
{
    assert(m_config_item == configured_item::control_stick ||
           m_config_item == configured_item::camera_stick);

    switch (m_joyconfig_stage) {
    case joyconfig_stage::none:
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
                mp_config_timer = new Timer(5000.0, false, [player,this](){
                    // The changed axis is the vertical axis
                    auto& plyconf = GameState::instance.config[player];
                    int axis = 0;
                    int axescount = 0;
                    float axislimit = 0.0f;
                    const float* joyaxes = glfwGetJoystickAxes(plyconf.joy_index, &axescount);
                    findChangedAxis(axescount, m_neutral_joyaxes[player].data(), joyaxes, axis, axislimit);

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
                mp_config_timer = new Timer(5000.0, false, [player,this](){
                    // The changed axis is the horizontal axis
                    auto& plyconf = GameState::instance.config[player];
                    int axis = 0;
                    int axescount = 0;
                    float axislimit = 0.0f;
                    const float* joyaxes = glfwGetJoystickAxes(plyconf.joy_index, &axescount);
                    findChangedAxis(axescount, m_neutral_joyaxes[player].data(), joyaxes, axis, axislimit);

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
                    plyconf.joy_dead_zone = max({m_neutral_joyaxes[player][plyconf.joy_horizontal.axisno],
                                                 m_neutral_joyaxes[player][plyconf.joy_vertical.axisno],
                                                 m_neutral_joyaxes[player][plyconf.joy_cam_horizontal.axisno],
                                                 m_neutral_joyaxes[player][plyconf.joy_cam_vertical.axisno]})
                                            + 0.1f;
                    //printf("Dead zone calculated as %.2f\n", plyconf.joy_dead_zone);

                    // End config stages
                    delete mp_config_timer;
                    mp_config_timer   = nullptr;
                    m_joyconfig_stage = joyconfig_stage::none;
                    m_config_item     = configured_item::none;
                    m_config_player   = -1;
                });
            }
        }

        ImGui::End();
        break;
    } // No default to have the compiler warn on missing items
}

void JoymenuScene::updateHatchConfig(int player)
{
    assert(m_config_item == configured_item::hatch);
    static string prompt;

    if (prompt.empty()) {
        prompt = _("Please press UP on the hatch.");
    }

    ImGui::SetNextWindowPos(ImVec2(100.0f, 100.0f));
    ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
    ImGui::Begin(_("Configuring hatch"), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
    ImGui::TextWrapped(prompt.c_str());
    ImGui::NewLine();
    ImGui::End();

    auto& plyconf = GameState::instance.config[player];
    int button    = findPressedButton(plyconf.joy_index);

    if (button != -1) {
        waitUntilButtonReleased(plyconf.joy_index, button);

        switch (m_hatchconfig_stage) {
        case hatchconfig_stage::up:
            plyconf.hatch_up    = button;
            m_hatchconfig_stage = hatchconfig_stage::right;
            prompt              = _("Please press RIGHT on the hatch.");
            break;
        case hatchconfig_stage::right:
            plyconf.hatch_right = button;
            m_hatchconfig_stage = hatchconfig_stage::down;
            prompt              = _("Please press DOWN on the hatch.");
            break;
        case hatchconfig_stage::down:
            plyconf.hatch_down  = button;
            m_hatchconfig_stage = hatchconfig_stage::left;
            prompt              = _("Please press LEFT on the hatch.");
            break;
        case hatchconfig_stage::left:
            plyconf.hatch_left  = button;
            m_hatchconfig_stage = hatchconfig_stage::none;
            m_config_item       = configured_item::none;
            m_config_player     = -1;
            prompt.clear();
            break;
        case hatchconfig_stage::none:
            assert(false);
            break;
        } // No default to warn about missing items
    }
}

void JoymenuScene::updateActionButtonConfig(int player)
{
    assert(m_config_item == configured_item::action_buttons);
    static string prompt;

    if (prompt.empty()) {
        prompt = _("Please press the UP action button.");
    }

    ImGui::SetNextWindowPos(ImVec2(100.0f, 100.0f));
    ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
    ImGui::Begin(_("Configuring action buttons"), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
    ImGui::TextWrapped(prompt.c_str());
    ImGui::NewLine();
    ImGui::End();

    auto& plyconf = GameState::instance.config[player];
    int button    = findPressedButton(plyconf.joy_index);

    if (button != -1) {
        waitUntilButtonReleased(plyconf.joy_index, button);

        switch (m_actbuttonconfig_stage) {
        case actbuttonconfig_stage::top:
            plyconf.actbutton_up    = button;
            m_actbuttonconfig_stage = actbuttonconfig_stage::right;
            prompt = _("Please press the RIGHT action button.");
            break;
        case actbuttonconfig_stage::right:
            plyconf.actbutton_right = button;
            m_actbuttonconfig_stage = actbuttonconfig_stage::bottom;
            prompt = _("Please press the DOWN action button.");
            break;
        case actbuttonconfig_stage::bottom:
            plyconf.actbutton_down  = button;
            m_actbuttonconfig_stage = actbuttonconfig_stage::left;
            prompt = _("Please press the LEFT action button.");
            break;
        case actbuttonconfig_stage::left:
            plyconf.actbutton_left  = button;
            m_actbuttonconfig_stage = actbuttonconfig_stage::none;
            m_config_item           = configured_item::none;
            m_config_player         = -1;
            prompt.clear();
            break;
        case actbuttonconfig_stage::none:
            assert(false);
            break;
        } // No default to warn about missing items
    }
}

void JoymenuScene::updateShoulderButtonConfig(int player)
{
    assert(m_config_item == configured_item::shoulder_buttons);
    static string prompt;

    if (prompt.empty()) {
        prompt = _("Please press the LEFT shoulder button.");
    }

    ImGui::SetNextWindowPos(ImVec2(100.0f, 100.0f));
    ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
    ImGui::Begin(_("Configuring shoulder buttons"), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
    ImGui::TextWrapped(prompt.c_str());
    ImGui::NewLine();
    ImGui::End();

    auto& plyconf = GameState::instance.config[player];
    int button    = findPressedButton(plyconf.joy_index);

    if (button != -1) {
        waitUntilButtonReleased(plyconf.joy_index, button);

        switch (m_shoulderbuttonconfig_stage) {
        case shoulderbuttonconfig_stage::left:
            plyconf.shoulderbutton_left  = button;
            m_shoulderbuttonconfig_stage = shoulderbuttonconfig_stage::right;
            prompt = _("Please press the RIGHT shoulder button.");
            break;
        case shoulderbuttonconfig_stage::right:
            plyconf.shoulderbutton_right = button;
            m_shoulderbuttonconfig_stage = shoulderbuttonconfig_stage::none;
            m_config_item                = configured_item::none;
            m_config_player              = -1;
            prompt.clear();
            break;
        case shoulderbuttonconfig_stage::none:
            assert(false);
            break;
        } // No default to warn about missing items
    }
}

void JoymenuScene::updateMenuButtonConfig(int player)
{
    assert(m_config_item == configured_item::menu_buttons);
    static string prompt;

    if (prompt.empty()) {
        prompt = _("Please press the desired MENU button.");
    }

    ImGui::SetNextWindowPos(ImVec2(100.0f, 100.0f));
    ImGui::SetNextWindowSize(ImVec2(300.0f, 200.0f));
    ImGui::Begin(_("Configuring menu buttons"), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
    ImGui::TextWrapped(prompt.c_str());
    ImGui::NewLine();
    ImGui::End();

    auto& plyconf = GameState::instance.config[player];
    int button    = findPressedButton(plyconf.joy_index);

    if (button != -1) {
        waitUntilButtonReleased(plyconf.joy_index, button);

        switch (m_menubuttonconfig_stage) {
        case menubuttonconfig_stage::menu:
            plyconf.menu_button  = button;
            m_menubuttonconfig_stage = menubuttonconfig_stage::hud;
            prompt = _("Please press the desired HUD toggle button.");
            break;
        case menubuttonconfig_stage::hud:
            plyconf.hud_button = button;
            m_menubuttonconfig_stage = menubuttonconfig_stage::none;
            m_config_item                = configured_item::none;
            m_config_player              = -1;
            prompt.clear();
            break;
        case menubuttonconfig_stage::none:
            assert(false);
            break;
        } // No default to warn about missing items
    }
}

void JoymenuScene::readNeutralPositions(int player)
{
    assert(player == PLAYER1 || player == PLAYER2);

    auto& plyconf = GameState::instance.config[player];
    int count     = 0;

    const float* joyaxes      = glfwGetJoystickAxes(plyconf.joy_index, &count);
    m_neutral_joyaxes[player] = vector(joyaxes, joyaxes + count);

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
