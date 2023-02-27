#ifndef ILMENDUR_HERO_HPP
#define ILMENDUR_HERO_HPP
#include "actor.hpp"

class Hero: public Actor
{
public:
    Hero(ObjectLayer* p_layer, int herono);
    ~Hero();

    virtual void update();
    void checkInput();
private:
    int m_herono;
};

#endif /* ILMENDUR_HERO_HPP */
