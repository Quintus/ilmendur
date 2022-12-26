#include "debug_map_scene.hpp"
#include "../camera.hpp"
#include "../actors/actor.hpp"
#include "../map.hpp"
#include "../audio.hpp"
#include "../actors/player.hpp"
#include "../ilmendur.hpp"
#include "../gui.hpp"

DebugMapScene::DebugMapScene(const std::string& map)
    : Scene(),
      mp_cam1(new Camera(*this, Ilmendur::instance().viewportPlayer1())),
      mp_cam2(new Camera(*this, Ilmendur::instance().viewportPlayer2())),
      mp_player(nullptr)
{
    mp_map = new Map(map);
    mp_cam1->setBounds(mp_map->drawRect());
    mp_cam2->setBounds(mp_map->drawRect());
    mp_cam1->setViewport(Ilmendur::instance().viewportPlayer1());
    mp_cam2->setViewport(Ilmendur::instance().viewportPlayer2());

    if (mp_map->backgroundMusic().empty()) {
        Ilmendur::instance().audioSystem().stopBackgroundMusic();
    } else {
        Ilmendur::instance().audioSystem().playBackgroundMusic(mp_map->backgroundMusic());
    }
}

DebugMapScene::~DebugMapScene()
{
    Ilmendur::instance().audioSystem().stopBackgroundMusic();
    delete mp_map;

    if (mp_cam1) {
        delete mp_cam1;
    }
    if (mp_cam2) {
        delete mp_cam2;
    }
}

void DebugMapScene::update()
{
    // Update all actors
    mp_map->update();

    // Centre camera on the player
    if (mp_player) {
        mp_cam1->setPosition(mp_player->position());
    }
    mp_cam2->setPosition(Vector2f(1600, 2600));
}

void DebugMapScene::draw(SDL_Renderer* p_stage)
{
    // Camera 1
    SDL_Rect camview = mp_cam1->view();
    mp_cam1->draw(p_stage);
    mp_map->draw(p_stage, &camview);

    // Camera 2
    camview = mp_cam2->view();
    mp_cam2->draw(p_stage);
    mp_map->draw(p_stage, &camview);
}

void DebugMapScene::handleKeyDown(const SDL_Event& event)
{
    Scene::handleKeyDown(event);

    switch (event.key.keysym.sym) {
    case SDLK_UP:
    case SDLK_RIGHT: // fall-through
    case SDLK_DOWN:  // fall-through
    case SDLK_LEFT:  // fall-through
        if (mp_player) {
            mp_player->checkInput();
        }
        break;
    default:
        // Ignore
        break;
    }
}

void DebugMapScene::handleKeyUp(const SDL_Event& event)
{
    Scene::handleKeyUp(event);
    static bool test = false;

    switch (event.key.keysym.sym) {
    case SDLK_UP:
    case SDLK_RIGHT: // fall-through
    case SDLK_DOWN:  // fall-through
    case SDLK_LEFT:  // fall-through
        if (mp_player) {
            mp_player->checkInput();
        }
        break;
    case SDLK_ESCAPE:
        Ilmendur::instance().popScene();
        break;
    case SDLK_z:
        // DEBUG: E.g. in NPC::activate()
        if (!test) {
            GUISystem::messageDialog(1, {"First Message", "Second Message"}, [&] {test = false;});
            test = true;
        }
        break;
    default:
        // Ignore
        break;
    }
}
