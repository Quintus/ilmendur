#include "gui.hpp"
#include "ilmendur.hpp"
#include "imgui/imgui.h"
#include "os.hpp"
#include "util.hpp"
#include <cassert>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <filesystem>

// Message box height in pixels
#define MSGBOX_HEIGHT 200

using namespace std;

namespace {
    class MessageDialog
    {
    public:
        MessageDialog(unsigned int playerno, std::initializer_list<std::string> texts)
            : m_playerno(playerno),
              m_texts(texts),
              m_current_text(0) {}

        void setCallback(std::function<void()> cb) {
            m_cb = cb;
        }

        void update() {
            assert(m_current_text < m_texts.size());

            SDL_Rect boxarea;
            if (m_playerno == 1) {
                boxarea = Ilmendur::instance().viewportPlayer1();
            } else if (m_playerno == 2) {
                boxarea = Ilmendur::instance().viewportPlayer2();
            } else { // Full-screen dialog
                boxarea = Ilmendur::instance().renderArea();
            }

            // Place the message dialog at the viewport bottom with 20 px margin.
            boxarea.x += 20;
            boxarea.w -= 40;
            boxarea.y = boxarea.h - MSGBOX_HEIGHT - 20;
            boxarea.h = MSGBOX_HEIGHT;

            ImGui::SetNextWindowPos(ImVec2(boxarea.x, boxarea.y));
            ImGui::SetNextWindowSize(ImVec2(boxarea.w, boxarea.h));
            ImGui::Begin("TextDialog", nullptr, ImGuiWindowFlags_NoDecoration);
            ImGui::TextWrapped(m_texts[m_current_text].c_str());
            ImGui::End();
        }

        bool next() {
            if (++m_current_text == m_texts.size()) {
                m_current_text = 0;
                m_cb();
                return false;
            }
            return true;
        }

    private:
        unsigned int m_playerno;
        std::vector<std::string> m_texts;
        size_t m_current_text;
        std::function<void()> m_cb;
    };
}

static std::map<string, ImFont*> s_fonts;
static vector<unique_ptr<MessageDialog>> s_active_elements;

void GUISystem::update()
{
    for(unique_ptr<MessageDialog>& p_el: s_active_elements) {
        p_el->update();
    }
}

bool GUISystem::handleEvent(const SDL_Event& event)
{
    if (event.type != SDL_KEYUP) {
        return false;
    }

    switch (event.key.keysym.sym) {
    case SDLK_RETURN:
        if (s_active_elements.empty()) {
            return false;
        }

        if (!s_active_elements.front()->next()) {
            s_active_elements.erase(s_active_elements.begin());
        }

        return true;
    default:
        // Ignore
        return false;
    }
}

void GUISystem::loadFonts()
{
    namespace fs = std::filesystem;

    ImGuiIO& io = ImGui::GetIO();
    fs::path fontpath(OS::gameDataDir() / fs::u8path("fonts") / fs::u8path("LinLibertine_R.otf"));
    ifstream file(fontpath, ifstream::in | ifstream::binary);
    string binary(READ_FILE(file));

    // Hand over to ImGui, which takes ownership of `buf'.
    char* buf = new char[binary.size()];
    memcpy(buf, binary.data(), binary.size());
    s_fonts["Default"] = io.Fonts->AddFontFromMemoryTTF(buf, binary.size(), 30.0f);
    assert(s_fonts["Default"]);

    buf = new char[binary.size()];
    memcpy(buf, binary.data(), binary.size());
    s_fonts["Small"]   = io.Fonts->AddFontFromMemoryTTF(buf, binary.size(), 22.0f);
    assert(s_fonts["Small"]);
}

void GUISystem::messageDialog(unsigned int playerno, initializer_list<string> texts, std::function<void()> callback)
{
    s_active_elements.emplace(s_active_elements.begin(),
                              new MessageDialog(playerno, texts));
    s_active_elements[0]->setCallback(callback);
}
