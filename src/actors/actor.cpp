#include "actor.hpp"
#include <iostream>

using namespace std;

Actor::Actor()
    : m_mass(0.0f),
      mp_scene_node(nullptr)
{
}

Actor::~Actor()
{
}

void Actor::collide(Actor& other)
{
    cout << this << " collided with " << &other << endl;
}
