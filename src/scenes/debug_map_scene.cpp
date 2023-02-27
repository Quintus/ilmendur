#include "debug_map_scene.hpp"
#include "../camera.hpp"
#include "../actors/actor.hpp"
#include "../map.hpp"
#include "../audio.hpp"
#include "../actors/hero.hpp"
#include "../ilmendur.hpp"
#include "../gui.hpp"
#include <cassert>

using namespace std;

DebugMapScene::DebugMapScene(const std::string& map)
    : Scene(),
      mp_cam1(new Camera(*this, Ilmendur::instance().viewportPlayer1())),
      mp_cam2(new Camera(*this, Ilmendur::instance().viewportPlayer2())),
      mp_freya(nullptr),
      mp_benjamin(nullptr),
      m_teleport_entry(-1)
{
    mp_map = new Map(map);
    mp_cam1->setBounds(mp_map->drawRect());
    mp_cam2->setBounds(mp_map->drawRect());
    mp_cam1->setViewport(Ilmendur::instance().viewportPlayer1());
    mp_cam2->setViewport(Ilmendur::instance().viewportPlayer2());

    if (mp_map->backgroundMusic().empty()) {
        Ilmendur::instance().audioSystem().stopBackgroundMusic();
    } else {
        //Ilmendur::instance().audioSystem().playBackgroundMusic(mp_map->backgroundMusic());
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

void DebugMapScene::setup()
{
    Scene::setup();
    mp_map->setup();

    if (m_teleport_entry > 0) {
        mp_map->makeHeroesTeleport(m_teleport_entry);
    } else {
        mp_map->makeHeroes();
    }

    mp_map->heroes(&mp_freya, &mp_benjamin);
}

void DebugMapScene::update()
{
    // Update all actors
    mp_map->update();

    // Centre camera on the hero
    if (mp_freya) {
        mp_cam1->setPosition(mp_freya->position());
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
        if (mp_freya) {
            mp_freya->checkInput();
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
        if (mp_freya) {
            mp_freya->checkInput();
        }
        break;
    case SDLK_ESCAPE:
        Ilmendur::instance().popScene();
        break;
    case SDLK_z:
        // DEBUG: E.g. in NPC::activate()
        if (!test) {
            GUISystem::messageDialog(1, GUISystem::text_velocity::normal, "John Doe",
                                     {"Lorem ipsum dolor sit amet, consetetur sadipscing elitr, <em>sed diam nonumy</em> eirmod tempor <em>invidunt</em> ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua.",
                                      "Second Message: Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna.",
                                      format("Third Message: >%s<", "Test format")},
                                     [&] {test = false;});
            test = true;
        }
        break;
    case SDLK_j: {
        vector<Actor*> adj = mp_map->findAdjascentActors(mp_freya, mp_freya->lookDirection());
        string result = "Found ";
        result += to_string(adj.size());
        result += " actors\n";
        for (Actor* p_act: adj) {
            result += "  ID: ";
            result += to_string(p_act->id());
            result += "\n";
        }
        GUISystem::systemMessage(result);
    } break;
    case SDLK_RETURN: {
        vector<Actor*> adj = mp_map->findAdjascentActors(mp_freya, mp_freya->lookDirection());
        if (adj.size() > 0) {
            adj[0]->interact(mp_freya);
        }
    } break;
    default:
        // Ignore
        break;
    }
}

void DebugMapScene::useEntry(int entry_id)
{
    assert(entry_id > 0);
    m_teleport_entry = entry_id;
}
