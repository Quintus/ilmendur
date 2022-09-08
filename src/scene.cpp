#include "scene.hpp"
#include "camera.hpp"
#include "actors/actor.hpp"
#include "map.hpp"
#include "actors/player.hpp"
#include "ilmendur.hpp"

using namespace std;

Scene::Scene()
    : mp_cam1(new Camera(*this, Ilmendur::instance().viewportPlayer1())),
      mp_cam2(new Camera(*this, Ilmendur::instance().viewportPlayer2())),
      mp_player(nullptr)
{
    // DEBUG: This should only be in a subclass probably
    mp_map = new Map("Oak Fortress");
    mp_cam1->setBounds(mp_map->drawRect());
    mp_cam2->setBounds(mp_map->drawRect());
    mp_cam1->setViewport(Ilmendur::instance().viewportPlayer1());
    mp_cam2->setViewport(Ilmendur::instance().viewportPlayer2());
}

Scene::~Scene()
{
    delete mp_map;

    if (mp_cam1) {
        delete mp_cam1;
    }
    if (mp_cam2) {
        delete mp_cam2;
    }
}

void Scene::update()
{
    // Update all actors
    mp_map->update();

    // Centre camera on the player
    if (mp_player) {
        mp_cam1->setPosition(mp_player->position());
    }
    mp_cam2->setPosition(Vector2f(1600, 2600));
}

void Scene::draw(SDL_Renderer* p_stage)
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
