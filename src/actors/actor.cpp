#include "actor.hpp"
#include "../ilmendur.hpp"
#include "../texture_pool.hpp"
#include "../scenes/scene.hpp"
#include "../map.hpp"
#include <SDL2/SDL.h>
#include <cassert>

#define TILEWIDTH 32

using namespace std;

/**
 * Constructs a new actor. New actors cannot be constructed outside
 * of a map layer: pass the target layer as `p_layer`. You can always
 * change the layer an actor is on later.
 *
 * The actor is constructed with the requested graphic, or, if no
 * graphic is requested, it will be invisible. The parameter
 * `graphic` takes the same values as TexturePool's [] operator.
 */
Actor::Actor(int id, ObjectLayer* p_layer, const string& graphic)
    : mp_texinfo(nullptr),
      m_current_frame(0),
      m_ani_mode(animation_mode::on_move),
      m_lookdir(direction::none),
      m_ani_ticks(0),
      m_id(id),
      mp_layer(p_layer),
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

    // Triggering this assert means that a non-existant graphics file was requested.
    if (!graphic.empty()) {
        assert(mp_texinfo);
    }
}

void Actor::setAnimationMode(animation_mode mode)
{
    m_ani_mode = mode;
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

/**
 * Returns the draw rectangle, in world coordinates.
 */
SDL_Rect Actor::drawRect() const
{
    SDL_Rect result;
    result.w = mp_texinfo->stridex;
    result.h = mp_texinfo->stridey;
    result.x = m_pos.x - mp_texinfo->origx;
    result.y = m_pos.y - mp_texinfo->origy;
    return result;
}

/**
 * Returns the collision rectangle, in world coordinates.
 * Unless overridden by a subclass, invisible actors do not
 * have a collision box; for them, this method returns a
 * rectangle with width and height set to zero and X and Y
 * set to the position().
 */
SDL_Rect Actor::collisionBox() const
{
    SDL_Rect result;
    if (mp_texinfo) {
        result.w = mp_texinfo->collw;
        result.h = mp_texinfo->collh;
        result.x = m_pos.x - mp_texinfo->origx + mp_texinfo->collx;
        result.y = m_pos.y - mp_texinfo->origy + mp_texinfo->colly;
    } else {
        result.w = 0;
        result.h = 0;
        result.x = m_pos.x;
        result.y = m_pos.y;
    }
    return result;
}

void Actor::draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview)
{
    if (!mp_texinfo) { // Invisible actor
        return;
    }

    SDL_Rect destrect = drawRect();
    // Do not draw this actor if it is not within the camera view
    if (!SDL_HasIntersection(&destrect, p_camview)) {
        return;
    }

    // TODO: Honor p_camview width and height for scaling purposes.
    destrect.x -= p_camview->x;
    destrect.y -= p_camview->y;

    SDL_Rect srcrect;
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

    // Note: Collision checks happen in Map::update().
}

/**
 * Handle an event. The default implementation does nothing; override
 * in subclasses. When handling collision events, be sure to check
 * if the antiCollide() method is useful to you.
 */
void Actor::handleEvent(const Event&)
{
}

/**
 * Called when an actor tries to “interact” with this actor.
 * Usually, this is one of the heroes trying to “speak to” or
 * otherwise activate this actor. The default implementation
 * does nothing.
 */
void Actor::interact(Actor*)
{
}

/**
 * Moves p_actor out of p_other. p_actor will not move anymore after
 * this method returns. p_other will not be moved. The two actors
 * will be exactly adjascent after the return from this method.
 * `intersect' takes the intersection rectangle of the collision
 * boxes of the two.
 */
void Actor::antiCollide(Actor* p_actor, const Actor* p_other, const SDL_Rect& intersect)
{
    // First, the easy cases: four cardinal directions.
    if (p_actor->m_movedir.x == 0.0f && p_actor->m_movedir.y < 0.0f) { // North
        p_actor->stopMoving();
        p_actor->m_pos.y += intersect.h;
    } else if (p_actor->m_movedir.x > 0.0f && p_actor->m_movedir.y == 0.0f) { // East
        p_actor->stopMoving();
        p_actor->m_pos.x -= intersect.w;
    } else if (p_actor->m_movedir.x == 0.0f && p_actor->m_movedir.y > 0.0f) { // South
        p_actor->stopMoving();
        p_actor->m_pos.y -= intersect.h;
    } else if (p_actor->m_movedir.x < 0.0f && p_actor->m_movedir.y == 0.0f) { // West
        p_actor->stopMoving();
        p_actor->m_pos.x += intersect.w;
    } else { // Something in between. Complicated. (Note: both actors might not be moving at all!)
        /* The below seems to work fairly well. A clean solution would
         * probably generalise into a proper vector-movement based
         * approach. */
        SDL_Rect collrect1 = p_actor->collisionBox();
        SDL_Rect collrect2 = p_other->collisionBox();

        if (intersect.h > intersect.w) {
            if (collrect1.x <= collrect2.x) {
                p_actor->m_pos.x -= intersect.w;
            } else {
                p_actor->m_pos.x += intersect.w;
            }
        } else {
            if (collrect1.y <= collrect2.y) {
                p_actor->m_pos.y -= intersect.h;
            } else {
                p_actor->m_pos.y += intersect.h;
            }
        }
        p_actor->stopMoving();

        // My prior attempt to do this properly follows:
        ///* Der kollidierende Aktor wird zunächst horizontal verschoben, bis
        // * er aus dem kollidierten Aktor entfernt wurde. Anschließend
        // * wird der Sinussatz angewandt, um die noch fehlende korrekte
        // * Y-Position des kollidierenden Aktors zu berechnen. Dabei darf
        // * die Einbeziehung des zweiten Winkels wegfallen, da er 90°
        // * beträgt und sin(90°)=1 gilt. Bewegt der Aktor sich auf der X-Achse,
        // * kann die Berechnung ganz entfallen. */
        //float angle = M_PI - p_actor->m_movedir.angleWith(Vector2f(0,1));
        //p_actor->stopMoving();
        //if (collrect1.x <= collrect2.x) { // Kollidierer kommt von links
        //    p_actor->m_pos.x -= intersect.w;
        //} else { // Kollidierer kommt von rechts
        //    p_actor->m_pos.x += intersect.w;
        //}
        //if (!float_equal(0.5*M_PI - angle, 0.0f)) { // 0.5π rad = 90°
        //    p_actor->m_pos.y += intersect.w / sinf(0.5*M_PI - angle);
        //}
        //
        //cout << "This gives an angle of " << (180.0f*angle)/M_PI << "° with the Y axis" << endl;
        //cout << "ID " << p_actor->m_id << " is now at (" << p_actor->m_pos.x << "|" << p_actor->m_pos.y << ")" << endl;        }
    }
}
