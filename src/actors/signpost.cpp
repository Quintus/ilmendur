#include "signpost.hpp"
#include "../event.hpp"
#include "../gui.hpp"
#include "player.hpp"

using namespace std;

Signpost::Signpost(int id, SDL_Rect collbox, vector<string> texts)
    : Actor(id),
      m_texts(texts)
{
    warp(Vector2f(collbox.x, collbox.y));
    m_collbox = collbox;
}

Signpost::~Signpost()
{
}

void Signpost::update()
{
    // The signpost does nothing on update.
}

void Signpost::draw(SDL_Renderer*, const SDL_Rect*)
{
    // Signposts are CURRENTLY DEBUG not drawn, they are invisible.
}

SDL_Rect Signpost::collisionBox() const
{
    return m_collbox;
}

void Signpost::handleEvent(const Event& event)
{
    // Signposts may not be walked through.
    if (event.type == Event::Type::collision) {
        antiCollide(event.data.coll.p_other, this, event.data.coll.intersect);
    }
}

void Signpost::interact(Actor* p_other)
{
    Player* p_player = dynamic_cast<Player*>(p_other);
    if (!p_player) {
        return;
    }

    // TODO: Check which one of the heroes it is. For now assume player 1
    GUISystem::messageDialog(1, GUISystem::text_velocity::instant, "Signpost", m_texts);
}
