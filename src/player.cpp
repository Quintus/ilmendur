#include "player.hpp"
#include "scene.hpp"

#define PLAYER_VELOCITY 80

using namespace std;

Player::Player(Scene& scene)
    : Actor(scene, "chars/spaceship.png")
{
    scene.setPlayer(this);
}

Player::~Player()
{
}

void Player::update()
{
    Actor::update();
}

void Player::go(godir dir)
{
    Vector2f vec;
    switch (dir) {
    case godir::n:
        vec = Vector2f(0.0f, -1.0f);
        turn(direction::up);
        break;
    case godir::ne:
        vec = Vector2f(1.0f, -1.0f).normalise();
        break;
    case godir::e:
        vec = Vector2f(1.0f, 0.0f);
        turn(direction::right);
        break;
    case godir::se:
        vec = Vector2f(1.0f, 1.0f).normalise();
        break;
    case godir::s:
        vec = Vector2f(0.0f, 1.0f);
        turn(direction::down);
        break;
    case godir::sw:
        vec = Vector2f(-1.0f, 1.0f).normalise();
        break;
    case godir::w:
        vec = Vector2f(-1.0f, 0.0f);
        turn(direction::left);
        break;
    case godir::nw:
        vec = Vector2f(-1.0f, -1.0f).normalise();
        break;
    } // No default so the compiler can warn about missing values

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
        moveTo(vec, PLAYER_VELOCITY);
    }
}
