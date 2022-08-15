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
