#include "orion-core/clock.h"

#include "orion-core/platform/unix/unix_platform.h"

#include <spdlog/spdlog.h>

#include <time.h>

namespace orion::detail
{
    HighResolutionClock::time_point HighResolutionClock::now()
    {
        struct timespec time;
        if (clock_gettime(CLOCK_MONOTONIC_RAW, &time) == -1) {
            SPDLOG_LOGGER_ERROR(unix::logger(), "{}", unix::errno_string());
            return HighResolutionClock::time_point{std::chrono::nanoseconds{0}};
        }
        return HighResolutionClock::time_point{std::chrono::seconds{time.tv_sec} + std::chrono::nanoseconds{time.tv_nsec}};
    }
} // namespace orion::detail
