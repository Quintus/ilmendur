#include "ui.hpp"
#include <OGRE/Overlay/OgreOverlayManager.h>
#include <OGRE/OgreTextureManager.h>
#include <iostream>

using namespace std;
using namespace UISystem;

static const int glyphs = 0x00250;

static ImFont* s_font = nullptr;

/**
 * Construct a new GUI engine instance. Ensure the Ogre overlay system
 * is enabled before you call this.
 */
GUIEngine::GUIEngine()
{
    mp_imgui_overlay = new Ogre::ImGuiOverlay();
    mp_imgui_overlay->setZOrder(300);
    mp_imgui_overlay->addFont("LinLibertine_R", "fonts");
    mp_imgui_overlay->show();
    Ogre::OverlayManager::getSingleton().addOverlay(mp_imgui_overlay);

}

GUIEngine::~GUIEngine()
{
    Ogre::OverlayManager::getSingleton().destroy(mp_imgui_overlay);
}

/**
 * Update UI system state. Call this every frame before creating any
 * UI elements.
 */
void GUIEngine::update()
{
    Ogre::ImGuiOverlay::NewFrame();
}
