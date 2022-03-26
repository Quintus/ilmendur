#ifndef ILMENDUR_UI_HPP
#define ILMENDUR_UI_HPP
#include <string>
#include "nuklear.h"

namespace UISystem {

    class GUIEngine {
    public:
        GUIEngine();
        ~GUIEngine();

        inline nk_context& getContext() { return m_nkcontext; }

        void draw();

        struct FontData;
    private:
        void buildFontAtlas();

        nk_context m_nkcontext;
        nk_user_font m_nkfont;
        FontData* mp_ft;
    };

    void testFreetype();
}

#endif /* ILMENDUR_UI_HPP */
