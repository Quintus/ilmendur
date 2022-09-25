#include "title_scene.hpp"
#include "debug_map_scene.hpp"
#include "../ilmendur.hpp"
#include "../actors/player.hpp"
#include "../map.hpp"
#include "../imgui/imgui.h"

TitleScene::TitleScene()
    : Scene()
{
}

TitleScene::~TitleScene()
{
}

void TitleScene::update()
{
    ImGui::SetNextWindowPos(ImVec2(20.0f, 20.0f));
    ImGui::SetNextWindowSize(ImVec2(1870.0f, 980.0f));
    ImGui::Begin("Title Menu", nullptr, ImGuiWindowFlags_NoDecoration);
    if (ImGui::Button("START", ImVec2(1000.0f, 100.0f))) {
        startGame();
    }
    if (ImGui::Button("Quit", ImVec2(1000.0f, 100.0f))) {
        quitGame();
    }
    ImGui::End();
}

void TitleScene::draw(SDL_Renderer*)
{
}

void TitleScene::startGame()
{
    DebugMapScene* p_testscene = new DebugMapScene("Oak Fortress");
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
