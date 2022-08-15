#ifndef ILMENDUR_PLAYER_HPP
#define ILMENDUR_PLAYER_HPP
#include "actor.hpp"

class Player: public Actor
{
public:
    Player(Scene& scene);
    ~Player();

    virtual void update();
    void checkInput();
};

#endif /* ILMENDUR_PLAYER_HPP */
