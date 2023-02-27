#include "hero.hpp"
#include "../scenes/scene.hpp"

#define HERO_ID 999999
#define HERO_VELOCITY 80

using namespace std;

Hero::Hero(ObjectLayer* p_layer, int herono)
    : Actor(HERO_ID, p_layer, "chars/spaceship.png"),
      m_herono(herono)
{
}

Hero::~Hero()
{
}

void Hero::update()
{
    Actor::update();
}

/**
 * Check the keyboard state for hero movement input.
 * Call this from the event loop.
 */
void Hero::checkInput()
{
    Vector2f vec;
    const uint8_t* keys = SDL_GetKeyboardState(nullptr);

    if (keys[SDL_SCANCODE_UP]) {
        if (keys[SDL_SCANCODE_RIGHT]) {
            vec = Vector2f(1.0f, -1.0f).normalise();
        } else if (keys[SDL_SCANCODE_LEFT]) {
            vec = Vector2f(-1.0f, -1.0f).normalise();
        } else { // Only up
            vec = Vector2f(0.0f, -1.0f);
            turn(direction::up);
        }
    } else if (keys[SDL_SCANCODE_DOWN]) {
        if (keys[SDL_SCANCODE_RIGHT]) {
            vec = Vector2f(1.0f, 1.0f).normalise();
        } else if (keys[SDL_SCANCODE_LEFT]) {
            vec = Vector2f(-1.0f, 1.0f).normalise();
        } else { // Only down
            vec = Vector2f(0.0f, 1.0f);
            turn(direction::down);
        }
    } else if (keys[SDL_SCANCODE_RIGHT]) { // Only right
        vec = Vector2f(1.0f, 0.0f);
        turn(direction::right);
    } else if (keys[SDL_SCANCODE_LEFT]) { // Only left
        vec = Vector2f(-1.0f, 0.0f);
        turn(direction::left);
    } else if (isMoving()) { // No arrow keys pressed, stop if not standing still already.
        stopMoving();
        return; // The rest is only for the movement operation.
    }

    if (isMoving()) {
        // If already moving, just alter the move direction
        m_movedir = vec;
    } else {
        // Otherwise start a new movement process
        if (vec.x == 0.0f) {
            vec.x = m_pos.x;
        } else {
            vec.x /= 0.0f;
        }
        if (vec.y == 0.0f) {
            vec.y = m_pos.y;
        } else {
            vec.y /= 0.0f;
        }
        moveTo(vec, HERO_VELOCITY);
    }
}
