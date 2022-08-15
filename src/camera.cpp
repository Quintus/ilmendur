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

    m_bounds.x = 0;
    m_bounds.y = 0;
    m_bounds.w = 0;
    m_bounds.h = 0;
}

Camera::~Camera()
{
}

void Camera::setView(const SDL_Rect& r)
{
    m_view = r;
}

/**
 * Sets the maximum camera bounds. The camera may not show anything
 * outside the given rectangle, which is in world coordinates. Any
 * call to setPosition() that would reveal something outside this
 * bounds will be clamped so that the camera only shows the outmost
 * place that is within `r`.
 */
void Camera::setBounds(const SDL_Rect& r)
{
    m_bounds = r;
}

/**
 * Center the camera on the position `pos`. This recalculates
 * the view rectangle accordingly, whose width and height are
 * not altered. If you want to alter that or want to specify
 * the upper left coördinate, use setView() instead.
 */
void Camera::setPosition(const Vector2f& pos)
{
    m_view.x = pos.x - m_view.w / 2.0f;
    m_view.y = pos.y - m_view.h / 2.0f;

    // Enforce camera bounds, if given
    if (m_bounds.w > 0) {
        if (m_view.x < m_bounds.x) {
            m_view.x = m_bounds.x;
        }
        if (m_view.x + m_view.w >= m_bounds.x + m_bounds.w) {
            m_view.x = m_bounds.x + m_bounds.w - m_view.w;
        }
        if (m_view.y < m_bounds.y) {
            m_view.y = m_bounds.y;
        }
        if (m_view.y + m_view.h >= m_bounds.y + m_bounds.h) {
            m_view.y = m_bounds.y + m_bounds.h - m_view.h;
        }
    }
}
