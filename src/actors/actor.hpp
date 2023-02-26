#ifndef ILMENDUR_ACTOR_HPP
#define ILMENDUR_ACTOR_HPP
#include "../util.hpp"
#include "../globals.hpp"
#include <cstdint>
#include <functional>
#include <string>
#include <SDL2/SDL.h>

struct TextureInfo;
class Scene;
class ObjectLayer;
struct Event;

class Actor
{
public:
    enum class animation_mode { never, on_move, always };

    Actor(int id, ObjectLayer* p_layer, const std::string& graphic = "");
    virtual ~Actor();

    virtual void update();
    virtual void draw(SDL_Renderer* p_stage, const SDL_Rect* p_camview);
    virtual void handleEvent(const Event& event);
    virtual void interact(Actor* p_other);

    ObjectLayer* mapLayer() { return mp_layer; }

    void setGraphic(const std::string& graphic);
    void setAnimationMode(animation_mode mode);

    bool isMoving();
    void moveTo(const Vector2f& targetpos, float velocity);
    void moveTo(const Vector2f& targetpos, std::function<float(uint64_t)> velfunc);
    void moveRelative(direction dir, int fields = 1);
    void moveForward(int fields = 1);
    void moveBackward(int fields = 1);
    void stopMoving();
    void warp(const Vector2f& targetpos);
    void turn(direction dir);

    SDL_Rect drawRect() const;
    virtual SDL_Rect collisionBox() const;

    inline int id() const { return m_id; }
    inline const Vector2f& position() const { return m_pos; }
    inline const Vector2f& moveDirection() const { return m_movedir; }
    inline direction lookDirection() const { return m_lookdir; }
    inline bool isInvisible() const { return !mp_texinfo; }
private:
    void move();
    void setFrame(unsigned int frameno);
    void nextFrame();

    TextureInfo* mp_texinfo;
    int m_current_frame;
    animation_mode m_ani_mode;
    direction m_lookdir;
    int m_ani_ticks;

protected:
    int m_id; ///< Map-wide unique ID of this actor.
    ObjectLayer* mp_layer; // Map layer the actor is on (this has an association to the map)
    Vector2f m_pos;
    Vector2f m_targetpos;
    Vector2f m_movedir;
    uint64_t m_move_start;
    float m_passed_distance;
    float m_total_distance;
    std::function<float(uint64_t)> m_velfunc;

    static void antiCollide(Actor* p_actor, const Actor* p_other, const SDL_Rect& intersect);

    // For collision checks and mp_map assocation ObjectLayer needs access
    friend class ObjectLayer;
    friend class Map;
};

#endif /* ILMENDUR_ACTOR_HPP */
