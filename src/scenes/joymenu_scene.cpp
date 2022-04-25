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
#include <iostream>

// Height of the images displayed for the control buttons, etc.
#define CTRL_WIDGET_HEIGHT 128.0f

using namespace std;
using namespace SceneSystem;

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

JoymenuScene::JoymenuScene()
    : Scene("Joystick configuration menu scene"),
      mp_ui_system(nullptr),
      m_crossedcircle_tex(0),
      m_crossedcircle_yellow_tex(0),
      m_steercross_tex(0),
      m_buttons_tex(0),
      m_shoulderbuttons_tex(0),
      mp_config_timer(nullptr),
      m_joyconfig_stage(joyconfig_stage::none)
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
    Core::Application::getSingleton().getWindow().getOgreRenderWindow()->addViewport(p_camera);

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

    Core::Application::getSingleton().getWindow().getOgreRenderWindow()->removeAllViewports();
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
    updateGamepadConfigUI(Core::FREYA);

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
                mp_config_timer = new Core::Timer(5000.0, false, [&](){
                    // TODO: query axis
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
        ImGui::TextWrapped(_("Determining vertical MOVEMENT axis. Please press UP until the next prompt appears."));
        ImGui::NewLine();

        if (mp_config_timer) {
            centreCursorForTextX("0");
            ImGui::Text("%.0f", 5.0f - mp_config_timer->passedTime());
        } else {
            centreCursorForTextX(_("Start"));
            if (ImGui::Button(_("Start"))) {
                mp_config_timer = new Core::Timer(5000.0, false, [&](){
                    // TODO: query axis
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
        ImGui::TextWrapped(_("Determining horizontal MOVEMENT axis. Please press RIGHT until this prompt disappears."));
        ImGui::NewLine();

        if (mp_config_timer) {
            centreCursorForTextX("0");
            ImGui::Text("%.0f", 5.0f - mp_config_timer->passedTime());
        } else {
            centreCursorForTextX(_("Start"));
            if (ImGui::Button(_("Start"))) {
                mp_config_timer = new Core::Timer(5000.0, false, [&](){
                    // TODO: query axis
                    delete mp_config_timer;
                    mp_config_timer = nullptr;
                    m_joyconfig_stage = joyconfig_stage::none;
                });
            }
        }

        ImGui::End();
        break;
    } // No default to have the compiler warn on missing items
}

void JoymenuScene::updateGamepadConfigUI(int player)
{
    // TODO: Honor "player"
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
    ImGui::SetNextWindowSize(ImVec2(620.0f, 700.0f));
    ImGui::Begin(_("Gamepad Configuration Player 1 (Freya)"), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginCombo("", _("Select gamepad to use"))) { // Returns true only if the element is opened
        for(int i=0; i < 16; i++) {
            if (glfwJoystickPresent(i)) {
                ImGui::Selectable((to_string(i+1) + ": " + glfwGetJoystickName(i)).c_str(), false);
            }
        }
        ImGui::EndCombo();
    }

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
    centreCursorForTextX("Y-");
    ImGui::Text("Y-");
    ImGui::TableSetColumnIndex(4);
    centreCursorForTextX("Y-");
    ImGui::Text("Y-");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    centreCursorForTextY();
    ImGui::Text("X-");
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
        m_joyconfig_stage = joyconfig_stage::neutral;
    }
    ImGui::TableSetColumnIndex(2);
    centreCursorForTextY();
    ImGui::Text("X+");
    ImGui::TableSetColumnIndex(3);
    centreCursorForTextY();
    ImGui::Text("X-");
    ImGui::TableSetColumnIndex(4);
    ImGui::Image(reinterpret_cast<ImTextureID>(m_crossedcircle_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
    ImGui::TableSetColumnIndex(5);
    centreCursorForTextY();
    ImGui::Text("X+");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    centreCursorForTextX("Y+");
    ImGui::Text("Y+");
    ImGui::TableSetColumnIndex(4);
    centreCursorForTextX("Y+");
    ImGui::Text("Y+");

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
