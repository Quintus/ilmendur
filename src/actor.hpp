#ifndef ILMENDUR_ACTOR_HPP
#define ILMENDUR_ACTOR_HPP
#include "util.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <SDL2/SDL.h>

struct TextureInfo;
class Scene;

class Actor
{
public:
    enum class direction { none, up, right, down, left };
    enum class animation_mode { never, on_move, always };

    Actor(Scene& scene, const std::string& graphic = "");
    virtual ~Actor();

    virtual void update();
    virtual void draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview);
    void setGraphic(const std::string& graphic);

    bool isMoving();
    void moveTo(const Vector2f& targetpos, float velocity);
    void moveTo(const Vector2f& targetpos, std::function<float(uint64_t)> velfunc);
    void stopMoving();
    void warp(const Vector2f& targetpos);
    void turn(direction dir);

    SDL_Rect drawRect() const;

    inline const Vector2f& position() const { return m_pos; }
    inline const Vector2f& moveDirection() const { return m_movedir; }
    inline direction lookDirection() const { return m_lookdir; }
private:
    void move();
    void setFrame(unsigned int frameno);
    void nextFrame();

    Scene& mr_scene;
    TextureInfo* mp_texinfo;
    int m_current_frame;
    animation_mode m_ani_mode;
    direction m_lookdir;
    int m_ani_ticks;

protected:
    Vector2f m_pos;
    Vector2f m_targetpos;
    Vector2f m_movedir;
    uint64_t m_move_start;
    float m_passed_distance;
    float m_total_distance;
    std::function<float(uint64_t)> m_velfunc;
};

#endif /* ILMENDUR_ACTOR_HPP */
