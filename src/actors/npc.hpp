#ifndef ILMENDUR_NPC_HPP
#define ILMENDUR_NPC_HPP
#include "actor.hpp"

class Player;

class NonPlayableCharacter: public Actor
{
public:
    NonPlayableCharacter(int id, ObjectLayer* p_layer, std::string graphic);
    virtual ~NonPlayableCharacter();

    virtual void update();
    virtual void handleEvent(const Event&);
    virtual void interact(Actor* p_other);

    void attachActivationFunction(std::function<void(NonPlayableCharacter*,Player*)> af);
private:
    std::function<void(NonPlayableCharacter*,Player*)> m_activation_function;
    SDL_Rect m_collbox;
};

#endif /* ILMENDUR_NPC_HPP */
