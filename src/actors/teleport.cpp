#include "teleport.hpp"
#include "../event.hpp"
#include "../scenes/debug_map_scene.hpp"
#include "../ilmendur.hpp"
#include "../map.hpp"
#include "../actors/player.hpp"
#include <cassert>

using namespace std;

Entry::Entry(int id, ObjectLayer* p_layer, direction enter_dir)
    : Actor(id, p_layer),
      m_enter_dir(enter_dir)
{
}

Entry::~Entry()
{
}

direction Entry::enterDirection() const
{
    return m_enter_dir;
}

Teleport::Teleport(int id, ObjectLayer* p_layer, SDL_Rect box, int target_entry_id, string target_map_name)
    : Actor(id, p_layer),
      m_target_entry_id(target_entry_id),
      m_target_map_name(target_map_name)
{
    warp(Vector2f(box.x, box.y));
    m_collbox = box;
}

Teleport::~Teleport()
{
}

void Teleport::update()
{
}

void Teleport::draw(SDL_Renderer*, const SDL_Rect*)
{
    // Invisible
}

SDL_Rect Teleport::collisionBox() const
{
    return m_collbox;
}

void Teleport::handleEvent(const Event& event)
{
    if (event.type == Event::Type::collision) {
        // TODO: Check if the other hero is also within the collision rectangle

        if (m_target_map_name.empty()) { // Teleport within the same map
            Scene& scene = Ilmendur::instance().currentScene();
            DebugMapScene* p_scene = dynamic_cast<DebugMapScene*>(&scene);
            assert(p_scene); // Can only call this from a map scene

            Actor* p_entry_actor = nullptr;
            assert(p_scene->map().findActor(m_target_entry_id, &p_entry_actor));
            Entry* p_entry = dynamic_cast<Entry*>(p_entry_actor);
            assert(p_entry);

            p_scene->mp_freya->warp(p_entry->position());
            p_scene->mp_freya->turn(p_entry->enterDirection());
            p_scene->mp_benjamin->warp(p_entry->position());
            p_scene->mp_benjamin->turn(p_entry->enterDirection());
        } else { // Teleport to another map
            DebugMapScene* p_newscene = new DebugMapScene(m_target_map_name);
            p_newscene->useEntry(m_target_entry_id);
            Ilmendur::instance().popScene();
            Ilmendur::instance().pushScene(p_newscene);
        }
    }
}
