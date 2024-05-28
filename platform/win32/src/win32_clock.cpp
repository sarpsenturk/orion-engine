#include "orion-platform/clock.h"

#include "orion-win32/win32_platform.h"

namespace orion::platform
{
    namespace
    {
        std::int64_t query_performance_frequency()
        {
            static const auto qpc_freq = []() {
                LARGE_INTEGER freq;
                QueryPerformanceFrequency(&freq);
                return freq;
            }();
            return qpc_freq.QuadPart;
        }

        std::int64_t query_performance_counter()
        {
            LARGE_INTEGER ticks;
            QueryPerformanceCounter(&ticks);
            return ticks.QuadPart;
        }
    } // namespace

    HighResolutionClock::time_point HighResolutionClock::now()
    {
        return time_point(duration(static_cast<rep>(query_performance_counter()) / query_performance_frequency()));
    }
} // namespace orion::platform
