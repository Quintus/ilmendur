#ifndef ILMENDUR_EVENT_HPP
#define ILMENDUR_EVENT_HPP
#include <SDL2/SDL.h>

class Actor;

struct CollisionEvent
{
    Actor* p_other;
    SDL_Rect intersect;
};

struct Event
{
    enum class Type {
        none = 0,
        collision
    };

    Type type;
    union {
        CollisionEvent coll;
    } data;
};

#endif /* ILMENDUR_EVENT_HPP */
