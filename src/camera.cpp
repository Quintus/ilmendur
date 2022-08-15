#include "camera.hpp"

using namespace std;

Camera::Camera(Scene& m_scene)
    : mr_scene(m_scene)
{
    m_view.x = 0;
    m_view.y = 0;
    // DEBUG: Hardcoded to window dimensions for now, change this!
    m_view.w = 640;
    m_view.h = 480;
}

Camera::~Camera()
{
}

void Camera::setView(const SDL_Rect& r)
{
    m_view = r;
}

void Camera::setPosition(const Vector2f& pos)
{
    m_view.x = pos.x - m_view.w / 2.0f;
    m_view.y = pos.y - m_view.h / 2.0f;
    // TODO: Honor edge of map
}
