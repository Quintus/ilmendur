#include "joymenu_scene.hpp"
#include "../core/application.hpp"
#include "../core/window.hpp"
#include "../core/i18n.hpp"
#include <OGRE/OgreTextureManager.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OGRE/RTShaderSystem/OgreRTShaderSystem.h>
#include <GLFW/glfw3.h>

using namespace std;
using namespace SceneSystem;

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

    ImGui::BeginTable("table", 6, ImGuiTableFlags_Borders|ImGuiTableFlags_SizingFixedFit);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("Steering");
    ImGui::TableSetColumnIndex(4);
    ImGui::Text("Camera");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("Y-");
    ImGui::TableSetColumnIndex(4);
    ImGui::Text("Y-");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("X-");
    ImGui::TableSetColumnIndex(1);
    ImGui::Image(reinterpret_cast<ImTextureID>(m_crossedcircle_tex), ImVec2(128.0f, 128.0f));
    ImGui::TableSetColumnIndex(2);
    ImGui::Text("X+");
    ImGui::TableSetColumnIndex(3);
    ImGui::Text("X-");
    ImGui::TableSetColumnIndex(4);
    ImGui::Image(reinterpret_cast<ImTextureID>(m_crossedcircle_tex), ImVec2(128.0f, 128.0f));
    ImGui::TableSetColumnIndex(5);
    ImGui::Text("X+");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("Y+");
    ImGui::TableSetColumnIndex(4);
    ImGui::Text("Y+");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("Items/Action");
    ImGui::TableSetColumnIndex(4);
    ImGui::Text("Spells");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("6");
    ImGui::TableSetColumnIndex(4);
    ImGui::Text("1");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("7");
    ImGui::TableSetColumnIndex(1);
    ImGui::Image(reinterpret_cast<ImTextureID>(m_steercross_tex), ImVec2(128.0f, 128.0f));
    ImGui::TableSetColumnIndex(2);
    ImGui::Text("5");
    ImGui::TableSetColumnIndex(3);
    ImGui::Text("2");
    ImGui::TableSetColumnIndex(4);
    ImGui::Image(reinterpret_cast<ImTextureID>(m_buttons_tex), ImVec2(128.0f, 128.0f));
    ImGui::TableSetColumnIndex(5);
    ImGui::Text("3");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("1");
    ImGui::TableSetColumnIndex(4);
    ImGui::Text("4");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("Attack/Defence");
    ImGui::TableSetColumnIndex(4);
    ImGui::Text("Other");

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("5");
    ImGui::TableSetColumnIndex(1);
    ImGui::Image(reinterpret_cast<ImTextureID>(m_shoulderbuttons_tex), ImVec2(128.0f, 128.0f));
    ImGui::TableSetColumnIndex(2);
    ImGui::Text("6");
    ImGui::TableSetColumnIndex(4);
    ImGui::Text("[MENU]");
    ImGui::Text("[HUD]");
    ImGui::TableSetColumnIndex(5);
    ImGui::Text("9");
    ImGui::Text("8");
    ImGui::EndTable();

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
