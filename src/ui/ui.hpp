#ifndef ILMENDUR_UI_HPP
#define ILMENDUR_UI_HPP
#include <OGRE/Overlay/OgreImGuiOverlay.h>

namespace UISystem {

    class GUIEngine {
    public:
        GUIEngine();
        ~GUIEngine();

        void update();
    private:
        Ogre::ImGuiOverlay* mp_imgui_overlay;
    };
}

#endif /* ILMENDUR_UI_HPP */
