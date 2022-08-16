#ifndef ILMENDUR_CAMERA_HPP
#define ILMENDUR_CAMERA_HPP
#include "util.hpp"
#include <SDL2/SDL.h>

class Scene;

class Camera
{
public:
    Camera(Scene& m_scene, const SDL_Rect& initial_view);
    ~Camera();

    void setView(const SDL_Rect& r);
    inline const SDL_Rect& view() { return m_view; }
    inline const SDL_Rect& viewport() { return m_viewport; }

    void draw(SDL_Renderer* p_renderer);

    void setPosition(const Vector2f& pos);
    void setBounds(const SDL_Rect& r);
    void setViewport(const SDL_Rect& viewport);
private:
    Scene& mr_scene;
    SDL_Rect m_view;
    SDL_Rect m_bounds;
    SDL_Rect m_viewport;
};

#endif /* ILMENDUR_CAMERA_HPP */
