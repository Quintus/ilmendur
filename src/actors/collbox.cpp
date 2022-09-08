#include "collbox.hpp"
#include "../event.hpp"
#include <cassert>

using namespace std;

CollisionBox::CollisionBox(int id, SDL_Rect box)
    : Actor(id)
{
    warp(Vector2f(box.x, box.y));
    m_collbox = box;
}

CollisionBox::~CollisionBox()
{
}

void CollisionBox::update()
{
    // The collision box does nothing on update.
}

void CollisionBox::draw(SDL_Renderer*, const SDL_Rect*)
{
    // Collision boxes are not drawn, they are invisible.
}

SDL_Rect CollisionBox::collisionBox() const
{
    return m_collbox;
}

void CollisionBox::handleEvent(const Event& event)
{
    if (event.type == Event::Type::collision) {
        antiCollide(event.data.coll.p_other, this, event.data.coll.intersect);
    }
}
