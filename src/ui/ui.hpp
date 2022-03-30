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
        struct OpenGLData;
        struct Vertex;
    private:
        void buildFontAtlas();
        void makeNullTexture();
        void compileShaders();
        void uploadVertices(nk_convert_config* p_cfg);

        nk_context m_nkcontext;
        nk_user_font m_nkfont;
        nk_buffer m_nkcmds;
        nk_buffer m_nkvertices;
        nk_buffer m_nkelements;
        FontData* mp_ft;
        OpenGLData* mp_ogl;
    };

    void testFreetype();
}

#endif /* ILMENDUR_UI_HPP */
