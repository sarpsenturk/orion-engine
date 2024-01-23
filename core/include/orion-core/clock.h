#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>

namespace orion
{
    namespace platform
    {
        std::int64_t query_performance_frequency();
        std::int64_t query_performance_counter();
    } // namespace platform

    namespace detail
    {
        class QPCClock
        {
        public:
            using rep = std::int64_t;
            using period = std::micro;
            using duration = std::chrono::duration<rep, period>;
            using time_point = std::chrono::time_point<QPCClock, duration>;
            static constexpr auto is_steady = true;

            static time_point now();
        };
    } // namespace detail

    using Clock = detail::QPCClock;
    using FrameTime = typename Clock::duration;
} // namespace orion
