#include "camera.hpp"
#include <cassert>

using namespace std;

Camera::Camera(Scene& m_scene, const SDL_Rect& initial_view)
    : mr_scene(m_scene)
{
    m_view = initial_view;
    assert(initial_view.w != 0);
    assert(initial_view.h != 0);

    // Setting the width components to zero makes callers ignore width
    // and height restrictions.

    m_bounds.x = 0;
    m_bounds.y = 0;
    m_bounds.w = 0;
    m_bounds.h = 0;

    m_viewport.x = 0;
    m_viewport.y = 0;
    m_viewport.w = 0;
    m_viewport.h = 0;
}

Camera::~Camera()
{
}

void Camera::setView(const SDL_Rect& r)
{
    m_view = r;
}

void Camera::setViewport(const SDL_Rect& r)
{
    m_viewport = r;
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

void Camera::draw(SDL_Renderer* p_renderer)
{
    if (m_viewport.w > 0) {
        SDL_RenderSetViewport(p_renderer, &m_viewport);
    }
}
