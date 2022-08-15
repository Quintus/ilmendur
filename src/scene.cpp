#include "scene.hpp"
#include "camera.hpp"
#include "actor.hpp"
#include "map.hpp"

using namespace std;

Scene::Scene()
    : mp_cam1(new Camera(*this)),
      mp_cam2(new Camera(*this))
{
    // DEBUG: This should only be in a subclass probably
    mp_map = new Map("Oak Fortress");
}

Scene::~Scene()
{
    for(Actor* p_actor: m_actors) {
        delete p_actor;
    }

    delete mp_map;

    if (mp_cam1) {
        delete mp_cam1;
    }
    if (mp_cam2) {
        delete mp_cam2;
    }
}

/**
 * Register the given actor with this scene as a toplevel actor; the
 * Scene object takes ownership of the object. That is, when the scene
 * is destroyed, the Actor is as well. Do not free manually anymore.
 */
void Scene::addActor(Actor* p_actor)
{
    m_actors.push_back(p_actor);
}

void Scene::update()
{
    /* Note: This is not the place to optimise by trying to only update
     * actors within the camera range. That would cause rather unnatural
     * behaviour. Always update all actors on the stage, and optimise by
     * not drawing them all in draw(). */
    for (Actor* p_actor: m_actors) {
        p_actor->update();
    }

    //SDL_Rect camview = mp_cam1->getView();
    //camview.x += 1;
    //camview.y += 1;
    //mp_cam1->setView(camview);
}

void Scene::draw(SDL_Renderer* p_stage)
{
    // For now only utilise camera 1
    SDL_Rect camview = mp_cam1->getView();

    mp_map->draw(p_stage, &camview);
    for (Actor* p_actor: m_actors) {
        p_actor->draw(p_stage, &camview);
    }
}
