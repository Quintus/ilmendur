#include "timer.hpp"
#include "ilmendur.hpp"

using namespace std;

/**
 * Creates a new timer. The timer starts ticking immediately.
 *
 * \param[ms]
 * Timer interval in milliseconds.
 *
 * \param[repeat]
 * Whether the timer stops ticking after the timer interval has passed.
 *
 * \param[cb]
 * Callback to execute after the timer interval has passed.
 */
Timer::Timer(float ms, bool repeat, function<void()> cb)
    : m_cb(cb),
      m_ticks(0),
      m_interval((ms / 1000.0f) * ILMENDUR_TARGET_FRAMERATE), // Target framerate is in seconds, thus convert the milliseconds to seconds here
      m_repeat(repeat),
      m_ticking(true)
{
}

/**
 * Destroys the timer. No further calls to the timer callback
 * will happen, as it calls stop().
 */
Timer::~Timer()
{
    stop();
}

/**
 * Checks if the timer interval has passed and executes the callback if so.
 * Call this function once per frame.
 */
void Timer::update()
{
    if (m_ticking) {
        if (++m_ticks >= m_interval) {
            m_cb();
            m_ticks = 0;
            if (!m_repeat) {
                m_ticking = false;
            }
        }
    }
}

/**
 * Freeze the timer. Any amount of time elapsed is stored;
 * subsequently calling start() will continue at exactly the amount of
 * time that has already passed.
 */
void Timer::stop()
{
    m_ticking = false;
}

/**
 * Continue ticking after the timer was stopped with stop().
 */
void Timer::start()
{
    m_ticking = true;
}
