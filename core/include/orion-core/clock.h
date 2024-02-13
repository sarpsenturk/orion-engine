#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>

namespace orion
{
    namespace detail
    {
        class HighResolutionClock
        {
        public:
            using rep = std::int64_t;
            using period = std::nano;
            using duration = std::chrono::duration<rep, period>;
            using time_point = std::chrono::time_point<HighResolutionClock, duration>;
            static constexpr auto is_steady = true;

            static time_point now();
        };
    } // namespace detail

    using Clock = detail::HighResolutionClock;
    using FrameTime = typename Clock::duration;
} // namespace orion
