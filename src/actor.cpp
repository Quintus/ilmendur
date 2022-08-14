#include "actor.hpp"
#include "ilmendur.hpp"
#include "texture_pool.hpp"
#include <cassert>
#include <SDL2/SDL.h>


#include <iostream>

#define TILEWIDTH 32

using namespace std;

Actor::Actor(const string& graphic)
    : mp_texinfo(nullptr),
      m_current_frame(0),
      m_ani_mode(animation_mode::on_move),
      m_lookdir(direction::none),
      m_ani_ticks(0),
      m_move_start(0),
      m_passed_distance(0.0f),
      m_total_distance(0.0f)
{
    setGraphic(graphic);
}

Actor::~Actor()
{
}

/**
 * Set this actor to the given graphic. The graphics path is as per
 * TexturePool's [] operator.
 */
void Actor::setGraphic(const string& graphic)
{
    if (graphic.empty()) {
        mp_texinfo = nullptr;
    } else {
        mp_texinfo = Ilmendur::instance().texturePool()[graphic];
    }
}

bool Actor::isMoving()
{
    return m_move_start != 0;
}

/**
 * Request this actor to move to the position `targetpos`. `velocity` gives
 * the moving speed in pixels per second. For reference, a value of 32.0f
 * will make the actor cross one field in one second.
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
 *
 * It is possible to pass INFINITY as members of `targetpos`. In that case,
 * the actor is going to continue moving until stopMoving() is called. Beware
 * that your `velfunc` should handle this case then. Do not pass INFINITY for
 * only one component, unless that other component is zero. Doing so will
 * cause an exception when the function tries to normalise the vector.
 */
void Actor::moveTo(const Vector2f& targetpos, function<float(uint64_t)> velfunc)
{
    if (m_pos == targetpos) {
        return;
    }

    Vector2f translation(targetpos.x - m_pos.x, targetpos.y - m_pos.y);
    m_passed_distance = 0.0f;
    m_total_distance  = translation.length();
    m_movedir         = translation.normalise();
    m_targetpos       = targetpos;
    m_move_start      = SDL_GetTicks64();
    m_velfunc         = velfunc;

    if (m_lookdir != direction::none) { // Actors that do not look anywhere do not need their look direction to be changed.
        float xdist = targetpos.x - m_pos.x;
        float ydist = targetpos.y - m_pos.y;

        if (!float_equal(fabs(xdist), fabs(ydist))) { // Do not change lookdir if the X and Y distance covered are about equal.
            if (fabs(xdist) > fabs(ydist)) {
                if (xdist > 0) {
                    m_lookdir = direction::right;
                } else {
                    m_lookdir = direction::left;
                }
            } else if (fabs(xdist) < fabs(ydist)) {
                if (ydist > 0) {
                    m_lookdir = direction::down;
                } else {
                    m_lookdir = direction::up;
                }
            }
        }
    }
}

/**
 * Forcibly stop the actor’s current movement, if any, right now.
 * Any values passed to the moveTo() methods are discarded, to
 * continue movement, you will need to make another call to
 * moveTo().
 */
void Actor::stopMoving()
{
    std::function<float(uint64_t)> emptyfunc;
    m_velfunc.swap(emptyfunc); // Clear the function object
    m_movedir.clear();
    m_targetpos.clear();
    m_move_start      = 0;
    m_passed_distance = 0.0f;
    m_total_distance  = 0.0f;

    // Ensure the animation always ends with the straight up graphic,
    // unless this actor is permanently animated anyway.
    if (m_ani_mode == animation_mode::on_move) {
        setFrame(0);
    }
}

void Actor::turn(direction dir)
{
    m_lookdir = dir;
}

/**
 * Immediately warps this actor to the target position, not playing
 * any kind of move animation.
 */
void Actor::warp(const Vector2f& targetpos)
{
    m_pos = targetpos;
}

void Actor::setFrame(unsigned int frameno)
{
    m_current_frame = frameno;
}

void Actor::nextFrame()
{
    if (++m_current_frame >= mp_texinfo->frames) {
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
        if (++m_ani_ticks >= ILMENDUR_TARGET_FRAMERATE * (mp_texinfo->animation_time / 1000.0)) {
            nextFrame();
            m_ani_ticks = 0;
        }
        break;
    case animation_mode::on_move:
        if (isMoving()) {
            if (++m_ani_ticks >= ILMENDUR_TARGET_FRAMERATE * (mp_texinfo->animation_time / 1000.0)) {
                nextFrame();
                m_ani_ticks = 0;
            }
        }
        break;
    case animation_mode::never:
        break;
    } // No default to provoke compiler warnings on missing elements
}

void Actor::draw(SDL_Renderer* p_stage)
{
    if (!mp_texinfo) { // Invisible actor
        return;
    }

    SDL_Rect srcrect;
    SDL_Rect destrect;

    destrect.w = mp_texinfo->stridex;
    destrect.h = mp_texinfo->stridey;
    destrect.x = m_pos.x - mp_texinfo->origx;
    destrect.y = m_pos.y - mp_texinfo->origy;

    srcrect.x  = m_current_frame * mp_texinfo->stridex;
    srcrect.w  = mp_texinfo->stridex;
    srcrect.h  = mp_texinfo->stridex;
    if (mp_texinfo->stridey == mp_texinfo->height) {
        srcrect.y = 0;
    } else {
        switch (m_lookdir) {
        case direction::none:
        case direction::up: // fall-through
            srcrect.y = 0;
            break;
        case direction::right:
            srcrect.y = mp_texinfo->stridey;
            break;
        case direction::down:
            srcrect.y = mp_texinfo->stridey * 2;
            break;
        case direction::left:
            srcrect.y = mp_texinfo->stridey * 3;
            break;
        }
    }

    SDL_RenderCopy(p_stage, mp_texinfo->p_texture, &srcrect, &destrect);
}

void Actor::move()
{
    float distance_per_frame = m_velfunc(SDL_GetTicks64() - m_move_start) / static_cast<float>(ILMENDUR_TARGET_FRAMERATE);
    m_passed_distance += distance_per_frame;

    if (m_passed_distance >= m_total_distance) {
        m_pos = m_targetpos;
        stopMoving();
    } else {
        Vector2f translation = m_movedir * distance_per_frame;
        m_pos.x += translation.x;
        m_pos.y += translation.y;
    }
}
