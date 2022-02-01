#include "timer.hpp"
#include "application.hpp"

using namespace std;
using namespace Core;

/**
 * Constructs a new timer.
 *
 * \param duration
 * Amount of milliseconds to wait before the timer fires.
 *
 * \param repeat
 * Whether or not the timer will fire more than once.
 *
 * \param[in] cb
 * Callback function to execute when the time has come.
 *
 * \returns A new Timer instance.
 */
Timer::Timer(float duration, bool repeat, std::function<void()> cb)
    : m_frequency(1000.0f / duration),
      m_frames(0),
      m_repeat(repeat),
      m_fired(false),
      m_cb(cb)
{
}

/**
 * Update the timer’s internal state. Call this once per frame.
 * When the configured time has passed, it will execute the
 * stored callback. Subsequent calls to update() are a no-nop,
 * unless the timer has been configured a repeating timer.
 */
void Timer::update()
{
    if (m_fired) {
        return;
    }

    if (++m_frames > (Application::getSingleton().getFPS() / m_frequency)) {
        if (!m_repeat) {
            m_fired = true;
        }

        m_frames = 0;
        m_cb();
    }
}
