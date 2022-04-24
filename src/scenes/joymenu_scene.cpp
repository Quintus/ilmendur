#include "joymenu_scene.hpp"
#include "../core/application.hpp"
#include "../core/window.hpp"
#include "../core/i18n.hpp"
#include <OGRE/OgreTextureManager.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <GLFW/glfw3.h>

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
      m_steercross_tex(0),
      m_buttons_tex(0),
      m_shoulderbuttons_tex(0)
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
    Core::Application::getSingleton().getWindow().getOgreRenderWindow()->removeAllViewports();
    Ogre::RTShader::ShaderGenerator::getSingletonPtr()->removeSceneManager(mp_scene_manager);
    delete mp_ui_system;
}

void JoymenuScene::update()
{
    mp_ui_system->update();
    ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
    ImGui::SetNextWindowSize(ImVec2(620.0f, 700.0f));
    ImGui::Begin(_("Gamepad Configuration Player 1 (Freya)"), NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Gamepad to use:");

    if (ImGui::BeginCombo("", "Select gamepad")) { // Returns true only if the element is opened
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
    centreCursorForTextX("Steering");
    ImGui::Text("Steering");
    ImGui::TableSetColumnIndex(4);
    centreCursorForTextX("Camera");
    ImGui::Text("Camera");

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
    ImGui::Image(reinterpret_cast<ImTextureID>(m_crossedcircle_tex), ImVec2(CTRL_WIDGET_HEIGHT, CTRL_WIDGET_HEIGHT));
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
    centreCursorForTextX("Items/Action");
    ImGui::Text("Items/Action");
    ImGui::TableSetColumnIndex(4);
    centreCursorForTextX("Spells");
    ImGui::Text("Spells");

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
    centreCursorForTextX("Attack/Defence");
    ImGui::Text("Attack/Defence");
    ImGui::TableSetColumnIndex(4);
    centreCursorForTextX("Other");
    ImGui::Text("Other");

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
    ImGui::Text("[MENU]");
    ImGui::Text("[HUD]");
    ImGui::TableSetColumnIndex(5);
    ImGui::Text("9");
    ImGui::Text("8");
    ImGui::EndTable();

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Save Configuration").x - ImGui::GetStyle().FramePadding.x - 5);
    ImGui::Button("Save Configuration");

    ImGui::End();
    //ImGui::ShowDemoWindow();
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
