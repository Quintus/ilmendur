#include "actor.hpp"
#include "ilmendur.hpp"
#include <cassert>
#include <SDL2/SDL.h>

#define TILEWIDTH 32

using namespace std;

Actor::Actor()
    : m_current_frame(0),
      m_ani_mode(animation_mode::on_move),
      m_lookdir(direction::none),
      m_wait_time(1.0f),
      m_move_start(0),
      m_passed_distance(0.0f),
      m_total_distance(0.0f)
{
}

Actor::~Actor()
{
}

bool Actor::isMoving()
{
    return m_move_start != 0;
}

/**
 * Request this actor to move to the position `targetpos`. `velocity` gives
 * the moving speed in pixels per second.
 *
 * Under the hood, this function calls the more general moveTo()
 * overload with a constant velocity alteration function that always
 * returns `velocity`.
 */
void Actor::moveTo(const Vector2f& targetpos, float velocity)
{
    moveTo(targetpos, [velocity](uint64_t){return velocity;});
}

/**
 * This is the generalised version of moveTo(). It allows to use any
 * function to distribute the change of velocity over the movement
 * operation. The function takes time passed (in milliseconds) since
 * the call to moveTo() and should return the velocity of the actor in
 * that point in time, in pixels per second (or, to me more precise:
 * the length of the velocity vector). It is called once per frame, so
 * try to keep it performant.
 *
 * You want to use this version of moveTo() if the simple linear function
 * applied by the other moveTo() function is insufficient, for instance
 * because you want a stone to slowly stop rolling instead of progressing
 * further in a linear fashion. In that case, setting `velfunc` to something
 * involving e.g. the tanh() function could be useful.
 */
void Actor::moveTo(const Vector2f& targetpos, function<float(uint64_t)> velfunc)
{
    Vector2f translation(targetpos.x - m_pos.x, targetpos.y - m_pos.y);
    m_passed_distance = 0.0f;
    m_total_distance  = translation.length();
    m_movedir         = translation.normalise();
    m_targetpos       = targetpos;
    m_move_start      = SDL_GetTicks64();
    m_velfunc         = velfunc;
}

unsigned int Actor::frames()
{
    // TODO: Calculate image width / stridex
    return TILEWIDTH; // DEBUG!
}

void Actor::setFrame(unsigned int frameno)
{
    m_current_frame = frameno;
}

void Actor::nextFrame()
{
    if (++m_current_frame >= frames()) {
        m_current_frame = 0;
    }
}

void Actor::update()
{
    // Progress movement, if any
    if (isMoving()) {
        move();
    }

    // Update frame for animation if requested
    switch (m_ani_mode) {
    case animation_mode::always:
        // TODO: Honour m_wait_time
        nextFrame();
        break;
    case animation_mode::on_move:
        if (isMoving()) {
            float step = 1.0f / frames();
            float next_step = (m_current_frame + 1) * step;
            if (m_passed_distance >= next_step) {
                nextFrame();
            }
        }
        break;
    case animation_mode::never:
        break;
    } // No default to provoke compiler warnings on missing elements
}

void Actor::move()
{
    float distance_per_frame = m_velfunc(SDL_GetTicks64() - m_move_start) / static_cast<float>(ILMENDUR_TARGET_FRAMERATE);
    m_passed_distance += distance_per_frame;

    if (m_passed_distance >= m_total_distance) {
        m_pos = m_targetpos;

        std::function<float(uint64_t)> emptyfunc;
        m_velfunc.swap(emptyfunc); // Clear the function object
        m_move_start = 0;
        m_passed_distance = 0.0f;
        m_total_distance = 0.0f;
        m_movedir.clear();
        m_targetpos.clear();
        setFrame(0);
    } else {
        Vector2f translation = m_movedir * distance_per_frame;
        m_pos.x += translation.x;
        m_pos.y += translation.y;
    }
}
