#pragma once

#include <chrono>

namespace orion
{
    template<typename Clock = std::chrono::steady_clock>
        requires Clock::is_steady
    class Timer
    {
    public:
        Timer()
            : m_start(Clock::now())
        {
        }

        template<typename Duration = std::chrono::duration<double, std::milli>>
        auto elapsed() const
        {
            return std::chrono::duration_cast<Duration>(Clock::now() - m_start).count();
        }

    private:
        typename Clock::time_point m_start;
    };
} // namespace orion
