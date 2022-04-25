#ifndef ILMENDUR_TIMER_HPP
#define ILMENDUR_TIMER_HPP
#include <functional>

namespace Core {

    /**
     * Timer class for scheduling code to run after a specified period
     * of time has passed, either once or periodically. These timers
     * are not exact timers, but they are oriented towards the current
     * framerate. Therefore, it is necessary to call update() on them
     * once per frame; each of these calls is taken to be one frame
     * passed, increasing the internal counter. If that counter surpasses
     * the amount of frames calculated with the current framerate to be
     * the desired duration, the timer fires, i.e., invokes the configured
     * callback.
     *
     * Usage example:
     *
     *     static Timer t(500.0f, false, [](){cout << "Hello\n!";});
     *     t.update();
     *
     * This timer will wait for half a second and then print "Hello!"
     * to the console. After that, update() is a no-op.
     */
    class Timer {
    public:
        Timer(float duration, bool repeat, std::function<void()> cb);
        void update();
        float passedTime();
    private:
        float m_frequency;
        int m_frames;
        bool m_repeat;
        bool m_fired;
        std::function<void()> m_cb;
    };

}

#endif /* ILMENDUR_TIMER_HPP */
