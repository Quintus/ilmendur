#ifndef ILMENDUR_PLAYER_HPP
#define ILMENDUR_PLAYER_HPP
#include "actor.hpp"

class Player: public Actor
{
public:
    Player();
    ~Player();

    virtual void update();

    enum class godir { n, ne, e, se, s, sw, w, nw };
    void go(godir dir);
};

#endif /* ILMENDUR_PLAYER_HPP */
