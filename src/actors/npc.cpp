#include "npc.hpp"
#include "../event.hpp"
#include "hero.hpp"

NonPlayableCharacter::NonPlayableCharacter(int id, ObjectLayer* p_layer, std::string graphic)
    : Actor(id, p_layer, graphic)
{
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

void NonPlayableCharacter::interact(Actor* p_other)
{
    Hero* p_hero = dynamic_cast<Hero*>(p_other);
    if (!p_hero) {
        return;
    }

    m_activation_function(this, p_hero);
}

void NonPlayableCharacter::attachActivationFunction(std::function<void(NonPlayableCharacter*,Hero*)> af)
{
    m_activation_function = af;
}
