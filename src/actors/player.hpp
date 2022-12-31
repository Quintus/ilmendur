#ifndef ILMENDUR_PLAYER_HPP
#define ILMENDUR_PLAYER_HPP
#include "actor.hpp"

class Player: public Actor
{
public:
    Player(ObjectLayer* p_layer, int playerno);
    ~Player();

    virtual void update();
    void checkInput();
private:
    int m_playerno;
};

#endif /* ILMENDUR_PLAYER_HPP */
