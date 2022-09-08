#include "passage.hpp"
#include "../event.hpp"
#include "../map.hpp"
#include <cassert>

using namespace std;

Passage::Passage(int id, SDL_Rect area, pass_direction dir, const string& targetlayer)
    : Actor(id),
      m_size(area.w, area.h),
      m_passdir(dir),
      m_targetlayer(targetlayer)
{
    assert(m_passdir != 0);
    warp(Vector2f(area.x, area.y));

    // If no size is given, default to a small area of 10 pixels.
    // TODO: Should better be centred around m_pos.
    if (area.w == 0.0f) {
        area.w = 10.0f;
    }
    if (area.h == 0.0f)  {
        area.h = 10.0f;
    }
}

Passage::~Passage()
{
}

SDL_Rect Passage::collisionBox() const
{
    SDL_Rect box;
    box.x = m_pos.x;
    box.y = m_pos.y;
    box.w = m_size.x;
    box.h = m_size.y;

    return box;
}

void Passage::handleEvent(const Event& event)
{
    if (event.type != Event::Type::collision) {
        return;
    }

    // Activate passage if the move direction contains the target direction.
    const Vector2f movedir = event.data.coll.p_other->moveDirection();
    if ((movedir.y < 0.0f && m_passdir & Passage::up) ||
        (movedir.y > 0.0f && m_passdir & Passage::down) ||
        (movedir.x < 0.0f && m_passdir & Passage::left) ||
        (movedir.x > 0.0f && m_passdir & Passage::up)) {
        mp_map->changeActorLayer(event.data.coll.p_other, m_targetlayer);
    }
}
