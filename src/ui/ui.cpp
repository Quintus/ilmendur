#include "ui.hpp"
#include "../core/application.hpp"
#include "../core/window.hpp"
#include "../scenes/scene.hpp"
#include <GLFW/glfw3.h>
#include <OGRE/Overlay/OgreOverlaySystem.h>
#include <OGRE/Overlay/OgreOverlayManager.h>
#include <OGRE/OgreTextureManager.h>

using namespace std;
using namespace UISystem;

static UISystem::GUIEngine* sp_gui_engine = nullptr;

/**
 * Construct a new GUI engine instance. Ensure the Ogre overlay system
 * is enabled before you call this.
 */
GUIEngine::GUIEngine()
    : mp_imgui_overlay(new Ogre::ImGuiOverlay())
{
    if (sp_gui_engine) {
        throw(std::runtime_error("There can only be one instance of GUIEngine!"));
    }

    // General setup
    mp_imgui_overlay->setZOrder(300);
    mp_imgui_overlay->addFont("LinLibertine_R", "fonts");
    mp_imgui_overlay->show();
    Ogre::OverlayManager::getSingleton().addOverlay(mp_imgui_overlay);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));

    /* Since Ilmendur does not use OgreBrites, Ogre's SDL bridge,
     * Imgui needs some manual care here. Refer to the Imgui docs
     * for information and notably the imgui_impl_glfw.cpp example
     * file of Imgui. Ogre's Imgui overlay implementation also is
     * a useful source, just as Imgui's GLFW+OpenGL3 example. */

    ImGuiIO& io = ImGui::GetIO();
    // Setup clipboard callbacks. The ClipboardUserData pointer is
    // psased to the clipboard callbacks by Imgui.
    io.ClipboardUserData = Core::Application::getSingleton().getWindow().getGLFWWindow();
    io.GetClipboardTextFn = GUIEngine::getClipboardText;
    io.SetClipboardTextFn = GUIEngine::setClipboardText;

    io.MouseDrawCursor = true;

    // Input callbacks are registered in Application::run() as they are
    // relevant for more than just the UI. The scene is required to call
    // GUIEngine::process*() to forward inputs to Imgui.

#if defined(_WIN32)
    io.ImeWindowHandle = reinterpret_cast<void*>(glfwGetWin32Window(Core::Application::getSingleton().getWindow().getGLFWWindow()));
#endif

    // Tell Imgui which GLFW keys are which Imgui keys.
    io.KeyMap[ImGuiKey_Tab]         = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow]   = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow]  = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow]     = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow]   = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp]      = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown]    = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home]        = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End]         = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert]      = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete]      = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace]   = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space]       = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter]       = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape]      = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
    io.KeyMap[ImGuiKey_A]           = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C]           = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V]           = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X]           = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y]           = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z]           = GLFW_KEY_Z;
}

GUIEngine::~GUIEngine()
{
    Ogre::OverlayManager::getSingleton().destroy(mp_imgui_overlay);
    sp_gui_engine = nullptr;
}

GUIEngine& GUIEngine::getSingleton()
{
    return *sp_gui_engine;
}

/// Write callback for ImGUI's clipboard functionality.
void GUIEngine::setClipboardText(void* ptr, const char* text)
{
    glfwSetClipboardString(reinterpret_cast<GLFWwindow*>(ptr), text);
}

/// Read callback for ImGUI's clipboard functionality.
const char* GUIEngine::getClipboardText(void* ptr)
{
    return glfwGetClipboardString(reinterpret_cast<GLFWwindow*>(ptr));
}

/**
 * Allows the scene to draw UI elements. What this function actually does
 * is to enable Ogre's Overlay system, but as the overlay system is only
 * used for the UI elements, and no other Ogre overlays are used, this
 * is equivalent.
 */
void GUIEngine::enable(SceneSystem::Scene& scene)
{
    scene.getSceneManager().addRenderQueueListener(Ogre::OverlaySystem::getSingletonPtr());
}

/**
 * Update UI system state. Call this every frame before creating any
 * UI elements.
 */
void GUIEngine::update()
{
    // Ogre::ImGuiOverlay::NewFrame():
    // - sends the current window size to Imgui
    // - Calls ImGui::NewFrame()
    Ogre::ImGuiOverlay::NewFrame();
}

/**
 * Call this from your scene's processKeyInput() function and pass in
 * the GLFW key input arguments. This method returns true if Imgui
 * processed the input, and false otherwise. Usually you do not want
 * to process the key event further when Imgui has consumed it.
 */
bool GUIEngine::processKeyInput(int key, int scancode, int action, int mods)
{
    // FIXME: Ogre currently uses Imgui 1.85, which is a version still using
    // the old direct-write input API. Starting from Imgui 1.87, Imgui provides
    // an event-based input API, which should be preferred.
    ImGuiIO& io = ImGui::GetIO();

    // Skip unknown keys
    if (key < 0 || key >= IM_ARRAYSIZE(io.KeysDown)) {
        return false;
    }

    switch (action) {
    case GLFW_PRESS:
    case GLFW_REPEAT: // fall-through
        io.KeysDown[key] = true;
        break;
    case GLFW_RELEASE:
        io.KeysDown[key] = false;
        break;
    default: // May not happen (see glfw docs)
        assert(false);
    }

    io.KeyShift = (mods & GLFW_MOD_SHIFT)   == GLFW_MOD_SHIFT;
    io.KeyCtrl  = (mods & GLFW_MOD_CONTROL) == GLFW_MOD_CONTROL;
    io.KeyAlt   = (mods & GLFW_MOD_ALT)     == GLFW_MOD_ALT;
    io.KeySuper = (mods & GLFW_MOD_SUPER)   == GLFW_MOD_SUPER;

    return io.WantCaptureKeyboard;
}

bool GUIEngine::processCharInput(unsigned int codepoint)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddInputCharacter(codepoint);
    return io.WantCaptureKeyboard;
}

bool GUIEngine::processCursorMove(double xpos, double ypos)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MousePos.x = xpos;
    io.MousePos.y = ypos;
    return io.WantCaptureMouse;
}

bool GUIEngine::processMouseButton(int button, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (button) {
    case GLFW_MOUSE_BUTTON_LEFT:
        io.MouseDown[0] = action == GLFW_PRESS;
        break;
    case GLFW_MOUSE_BUTTON_RIGHT:
        io.MouseDown[1] = action == GLFW_PRESS;
        break;
    default:
        return false;
    }

    return io.WantCaptureMouse;
}
