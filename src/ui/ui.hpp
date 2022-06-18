#ifndef ILMENDUR_UI_HPP
#define ILMENDUR_UI_HPP
#include <OGRE/Overlay/OgreImGuiOverlay.h>

namespace SceneSystem {
    class Scene;
}

namespace UISystem {

    /**
     * The central component of the UI system. This class is meant to be instanciated and
     * used in scenes that need GUI elements, i.e. menus, labels, and other things directed
     * at the player. Construct it only *once* per scene. It builds upon ImGui, which does
     * not properly support multiple active "contexts" at once. The connection with Ogre
     * is made via Ogre's built-in Imgui overlay. Since Ilmendur does not use Ogre Bites,
     * the input wiring happens inside this class.
     *
     * In your scene's update() function, call GUIEngine::update() before drawing any UI
     * elements; the method effectively boils down to Imgui's NewFrame(). Also define
     * overrides for all process* functions exposed by the Scene class and forward those
     * inputs into the GUI engine instance with the corresponding methods below. All of
     * these methods will return true if Imgui processed the input, and false otherwise;
     * in case of a true result, you usually do not want to further process the event.
     */
    class GUIEngine {
    public:
        static GUIEngine& getSingleton();
        GUIEngine();
        ~GUIEngine();

        void enable(SceneSystem::Scene& target_scene);

        bool processKeyInput(int key, int scancode, int action, int mods);
        bool processCharInput(unsigned int codepoint);
        bool processCursorMove(double xpos, double ypos);
        bool processMouseButton(int button, int action, int mods);
        void update();
    private:
        static void setClipboardText(void* ptr, const char* text);
        static const char* getClipboardText(void* ptr);
        Ogre::ImGuiOverlay* mp_imgui_overlay;
    };
}

#endif /* ILMENDUR_UI_HPP */
