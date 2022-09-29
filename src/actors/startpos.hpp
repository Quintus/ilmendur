#ifndef ILMENDUR_STARTPOS_HPP
#define ILMENDUR_STARTPOS_HPP
#include "actor.hpp"

class StartPosition: public Actor
{
public:
    StartPosition(int id, Vector2f position, int hero_no);
    virtual ~StartPosition();

    Vector2f startpos;
    int herono;
};

#endif /* ILMENDUR_STARTPOS_HPP */
