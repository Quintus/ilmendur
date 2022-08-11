#ifndef ILMENDUR_ACTOR_HPP
#define ILMENDUR_ACTOR_HPP
#include "util.hpp"
#include <cstdint>
#include <functional>

class Actor
{
public:
    enum class direction { none, up, right, down, left };
    enum class animation_mode { never, on_move, always };

    Actor();
    virtual ~Actor();

    void update();

    unsigned int frames();
    bool isMoving();
    void moveTo(const Vector2f& targetpos, float velocity);
    void moveTo(const Vector2f& targetpos, std::function<float(uint64_t)> velfunc);

private:
    void move();
    void setFrame(unsigned int frameno);
    void nextFrame();

    unsigned int m_current_frame;
    animation_mode m_ani_mode;
    direction m_lookdir;
    float m_wait_time;

    Vector2f m_pos;
    Vector2f m_targetpos;
    Vector2f m_movedir;
    uint64_t m_move_start;
    float m_passed_distance;
    float m_total_distance;
    std::function<float(uint64_t)> m_velfunc;
};

#endif /* ILMENDUR_ACTOR_HPP */
