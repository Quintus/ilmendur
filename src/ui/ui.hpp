#ifndef ILMENDUR_UI_HPP
#define ILMENDUR_UI_HPP
#include <string>
#include "nuklear.h"

namespace UISystem {

    class GUIEngine {
    public:
        GUIEngine();
        ~GUIEngine();

        inline nk_context& getContext() { return m_context; }

        void draw();

    private:
        void buildFontAtlas();

        struct FontData;
        nk_context m_context;
        FontData* mp_ft;
    };

    void testFreetype();
}

#endif /* ILMENDUR_UI_HPP */
