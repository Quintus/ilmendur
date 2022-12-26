#include "gui.hpp"
#include "imgui/imgui.h"
#include <cassert>
#include <vector>
#include <memory>

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

            ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f));
            ImGui::SetNextWindowSize(ImVec2(1870.0f, 980.0f));
            ImGui::Begin("TextDialog", nullptr, ImGuiWindowFlags_NoDecoration);
            ImGui::Text(m_texts[m_current_text].c_str());
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

void GUISystem::messageDialog(unsigned int playerno, initializer_list<string> texts, std::function<void()> callback)
{
    s_active_elements.emplace(s_active_elements.begin(),
                              new MessageDialog(playerno, texts));
    s_active_elements[0]->setCallback(callback);
}
