#include "npc.hpp"
#include "../event.hpp"
#include "player.hpp"

NonPlayableCharacter::NonPlayableCharacter(int id, ObjectLayer* p_layer, SDL_Rect collbox, std::string graphic)
    : Actor(id, p_layer, graphic)
{
    warp(Vector2f(collbox.x, collbox.y));
    m_collbox = collbox;
}

NonPlayableCharacter::~NonPlayableCharacter()
{
}

void NonPlayableCharacter::update()
{
    Actor::update();
}

void NonPlayableCharacter::handleEvent(const Event& event)
{
    // NPCs may not be walked through
    if (event.type == Event::Type::collision) {
        antiCollide(event.data.coll.p_other, this, event.data.coll.intersect);
    }

    // TODO: And they should not walk through anything, nor off the map...
}

SDL_Rect NonPlayableCharacter::collisionBox() const
{
    return m_collbox;
}


void NonPlayableCharacter::interact(Actor* p_other)
{
    Player* p_player = dynamic_cast<Player*>(p_other);
    if (!p_player) {
        return;
    }

    m_activation_function(this, p_player);
}

void NonPlayableCharacter::attachActivationFunction(std::function<void(NonPlayableCharacter*,Player*)> af)
{
    m_activation_function = af;
}
