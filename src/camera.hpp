#ifndef ILMENDUR_CAMERA_HPP
#define ILMENDUR_CAMERA_HPP
#include "util.hpp"
#include <SDL2/SDL.h>

class Scene;

class Camera
{
public:
    Camera(Scene& m_scene);
    ~Camera();

    void setView(const SDL_Rect& r);
    inline const SDL_Rect& getView() { return m_view; }

    void setPosition(const Vector2f& pos);
private:
    Scene& mr_scene;
    SDL_Rect m_view;
};

#endif /* ILMENDUR_CAMERA_HPP */
