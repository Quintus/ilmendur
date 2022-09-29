#include "startpos.hpp"

StartPosition::StartPosition(int id, Vector2f position, int hero_no)
    : Actor(id),
      startpos(position),
      herono(hero_no)
{
}

StartPosition::~StartPosition()
{
}
