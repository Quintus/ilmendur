#include "gui.hpp"
#include "ilmendur.hpp"
#include "imgui/imgui.h"
#include "os.hpp"
#include "util.hpp"
#include "timer.hpp"
#include "texture_pool.hpp"
#include "audio.hpp"
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
        MessageDialog(unsigned int playerno, GUISystem::text_velocity vel, std::initializer_list<std::string> texts)
            : m_playerno(playerno),
              m_textvel(0),
              m_texts(texts),
              m_current_text(0),
              m_displayed_text_range(0),
              m_endmark_sound_played(false) {
            switch (vel) {
            case GUISystem::text_velocity::instant: // Instant text display
                m_displayed_text_range = string::npos;
                break;
            case GUISystem::text_velocity::slow:
                m_textvel = 100.0f;
                break;
            case GUISystem::text_velocity::normal:
                m_textvel = 20.0f;
                break;
            case GUISystem::text_velocity::fast:
                m_textvel = 5.0f;
                break;
            }

            if (m_textvel != 0.0f) {
                mp_timer = std::unique_ptr<Timer>(new Timer(m_textvel, true, [&]{
                    m_displayed_text_range++;
                }));
            }
        }

        void setCallback(std::function<void()> cb) {
            m_cb = cb;
        }

        void update() {
            assert(m_current_text < m_texts.size());
            if (mp_timer) {
                mp_timer->update();
            }

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
            ImGui::TextWrapped(m_texts[m_current_text].substr(0, m_displayed_text_range).c_str());
            ImGui::End();

            // Display the continuation marker and play the talk ending sound on the last text
            if (m_displayed_text_range >= m_texts[m_current_text].length()) {
                string terminator;
                if (m_current_text == m_texts.size() - 1) {
                    if (!m_endmark_sound_played) {
                        Ilmendur::instance().audioSystem().playSound(m_playerno == 2 ? "ui/talkfin2.ogg" : "ui/talkfin1.ogg", AudioSystem::channel::ui);
                        m_endmark_sound_played = true;
                    }

                    terminator = "ui/square.png";
                } else {
                    // No sound for continuing texts.
                    terminator = "ui/arrowdown.png";
                }

                ImGui::SetNextWindowPos(ImVec2(boxarea.x + boxarea.w - 52, boxarea.y + boxarea.h - 28));
                ImGui::SetNextWindowSize(ImVec2(50, 50));
                ImGui::Begin("TextDialogIcon", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
                ImGui::Image(Ilmendur::instance().texturePool()[terminator]->p_texture, ImVec2(32, 32));
                ImGui::End();
            }
        }

        bool next() {
            if (++m_current_text == m_texts.size()) { // Last text done
                m_current_text = 0;
                m_cb();
                mp_timer.reset();

                if (m_playerno == 2) {
                    // TODO: Create a higher-pitch version of talkendmark1 for player 2 and play that one based on `m_playerno'.
                    Ilmendur::instance().audioSystem().playSound("ui/talkendmark1.ogg", AudioSystem::channel::ui);
                } else { // Player 1 or full-screen dialogue
                    Ilmendur::instance().audioSystem().playSound("ui/talkendmark1.ogg", AudioSystem::channel::ui);
                }
                return false;
            } else { // Another text to display
                if (m_playerno == 2) {
                    Ilmendur::instance().audioSystem().playSound("ui/continue2.ogg", AudioSystem::channel::ui);
                } else { // Player 1 or full-screen dialogue
                    Ilmendur::instance().audioSystem().playSound("ui/continue1.ogg", AudioSystem::channel::ui);
                }
            }

            // Set timer for the appearing letters again for the new text
            if (m_textvel != 0.0f) {
                m_displayed_text_range = 0;
                mp_timer = std::unique_ptr<Timer>(new Timer(m_textvel, true, [&]{
                    m_displayed_text_range++;
                }));
            }

            return true;
        }

    private:
        unsigned int m_playerno;
        float m_textvel;
        std::vector<std::string> m_texts;
        size_t m_current_text;
        size_t m_displayed_text_range;
        std::function<void()> m_cb;
        unique_ptr<Timer> mp_timer;
        bool m_endmark_sound_played;
    };
}

static std::map<string, ImFont*> s_fonts;
static vector<unique_ptr<MessageDialog>> s_active_elements;

/**
 * Update the GUI system's internal state. Call this once per frame
 * from the main loop; do not call it from any scene.
 */
void GUISystem::update()
{
    for(unique_ptr<MessageDialog>& p_el: s_active_elements) {
        p_el->update();
    }
}

/**
 * Submit an event to the GUI system. Call in the main loop for
 * any events; do not call it from any scene.
 *
 * Returns true if the event was used by the GUI system,
 * false otherwise.
 */
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

/**
 * Load the OTF font files from the file system into the font atlas.
 */
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

/**
 * Display a message dialog to the user.
 *
 * \param[playerno]
 * Which player's viewport to use. 1 = player 1, 2 = player 2, any other value = fullscreen message box.
 *
 * \param[vel]
 * Text display velocity. See text_velocity for the possible values.
 *
 * \param[texts]
 * List of texts to display. Per message box, one of these texts is displayed. When the player
 * presses the continue button, the next one is displayed.
 *
 * \param[callback]
 * This callback is executed when all message boxes have been confirmed by the player.
 */
void GUISystem::messageDialog(unsigned int playerno, text_velocity vel, initializer_list<string> texts, std::function<void()> callback)
{
    s_active_elements.emplace(s_active_elements.begin(),
                              new MessageDialog(playerno, vel, texts));
    s_active_elements[0]->setCallback(callback);
}
