#include "startpos.hpp"

StartPosition::StartPosition(int id, ObjectLayer* p_layer, Vector2f position, int hero_no)
    : Actor(id, p_layer),
      startpos(position),
      herono(hero_no)
{
}

StartPosition::~StartPosition()
{
}
