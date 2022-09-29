#include "title_scene.hpp"
#include "debug_map_scene.hpp"
#include "../ilmendur.hpp"
#include "../actors/player.hpp"
#include "../map.hpp"
#include "../os.hpp"
#include "../imgui/imgui.h"
#include <filesystem>
#include <algorithm>

using namespace std;
namespace fs = std::filesystem;

TitleScene::TitleScene()
    : Scene(),
      m_chosen_map(0)
{
    readUserMapList();
}

TitleScene::~TitleScene()
{
}

void TitleScene::update()
{
    static bool mapmode = false;

    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f));
    ImGui::SetNextWindowSize(ImVec2(1870.0f, 980.0f));
    ImGui::Begin("Title Menu", nullptr, ImGuiWindowFlags_NoDecoration);

    if (mapmode) {
        if (ImGui::Button("Back")) {
            mapmode = false;
        } else {
            static string msg;
            if (msg.empty()) {
                msg = to_string(m_user_maps.size()) + " map(s) found in " + (OS::userDataDir() / fs::u8path("maps")).u8string() + ":";
            }

            ImGui::Text(msg.c_str());
            if (ImGui::BeginListBox("maps")) {
                for (size_t i=0; i < m_user_maps.size(); i++) {
                    const string& mapname = m_user_maps[i];
                    if (ImGui::Selectable(mapname.c_str(), m_chosen_map == i)) {
                        m_chosen_map = i;
                    }
                    if (m_chosen_map == i) {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndListBox();
            }

            if (ImGui::Button("Load map")) {
                // Not quite efficient way to strip the .tmx suffix,
                // but this menu entry is for debugging purposes anyway.
                startGame(fs::u8path(m_user_maps[m_chosen_map]).stem().u8string());
            }
        }
    } else {
        if (ImGui::Button("START", ImVec2(1000.0f, 100.0f))) {
            startGame("Oak Fortress");
        }
        if (ImGui::Button("Quit", ImVec2(1000.0f, 100.0f))) {
            quitGame();
        }
        if (ImGui::Button("Load map from disk", ImVec2(1000.0f, 100.0f))) {
            mapmode = true;
        }
    }
    ImGui::End();
}

void TitleScene::draw(SDL_Renderer*)
{
}

void TitleScene::startGame(const std::string& mapname)
{
    DebugMapScene* p_testscene = new DebugMapScene(mapname);
    Player* p = new Player();
    p->warp(Vector2f(1600, 2600));
    p->turn(Actor::direction::left);
    p_testscene->map().addActor(p, "chars");
    p_testscene->setPlayer(p);

    // No popping, so that terminating the main game scene returns to the title scene.
    Ilmendur::instance().pushScene(p_testscene);
}

void TitleScene::quitGame()
{
    // The title scene is always at the bottom of the scene stack,
    // so popping it will initiate game termination.
    Ilmendur::instance().popScene();
}

void TitleScene::readUserMapList()
{
    fs::path mapdir = OS::userDataDir() / fs::u8path("maps");
    if (!fs::exists(mapdir)) {
        fs::create_directories(mapdir);
    }

    if (!fs::is_directory(mapdir)) {
        throw(runtime_error(string(mapdir.u8string()) + " is not a directory"));
    }

    for (const fs::directory_entry& iter: fs::directory_iterator(mapdir)) {
        if (iter.path().extension() == fs::u8path(".tmx")) {
            m_user_maps.push_back(iter.path().filename().u8string());
        }
    }

    sort(m_user_maps.begin(), m_user_maps.end());
    m_chosen_map = 0;
}
