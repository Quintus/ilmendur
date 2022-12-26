#ifndef ILMENDUR_TIMER_HPP
#define ILMENDUR_TIMER_HPP
#include <functional>

/**
 * A timer that executes a callback after a certain delay of time.
 * These timers are run *from the main loop* in the main thread.
 * No new thread is created. That means, the execution will not
 * happen exactly at the time specified. Use a lambda as the callback
 * function to access outer variables, but be sure to not let them
 * go out of scope while the timer is active.
 *
 * It is necessary to call update() on a timer once per frame,
 * as that is the function that will invoke the timer callback
 * at the appropriate intervals.
 */
class Timer
{
public:
    Timer(float ms, bool repeat, std::function<void()> cb);
    ~Timer();

    void stop();
    void start();

    void update();
private:
    std::function<void()> m_cb;
    float m_ticks;
    float m_interval;
    bool m_repeat;
    bool m_ticking;
};

#endif /* ILMENDUR_TIMER_HPP */
