#ifndef ILMENDUR_UI_HPP
#define ILMENDUR_UI_HPP
#include <OGRE/Overlay/OgreImGuiOverlay.h>

namespace UISystem {

    class GUIEngine {
    public:
        GUIEngine();
        ~GUIEngine();

        bool processKeyInput(int key, int scancode, int action, int mods);
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
