#pragma once

#include <chrono>

namespace orion::platform
{
    class HighResolutionClock
    {
    public:
        using rep = float;
        using period = std::ratio<1>;
        using duration = std::chrono::duration<rep, period>;
        using time_point = std::chrono::time_point<HighResolutionClock, duration>;
        static constexpr auto is_steady = true;

        static time_point now();
    };
} // namespace orion::platform
