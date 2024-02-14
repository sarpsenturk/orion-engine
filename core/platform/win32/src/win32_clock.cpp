#include "orion-core/clock.h"

#include "orion-win32/win32_platform.h"

namespace orion::platform
{
    std::int64_t query_performance_frequency()
    {
        LARGE_INTEGER freq;
        QueryPerformanceFrequency(&freq);
        return freq.QuadPart;
    }

    std::int64_t query_performance_counter()
    {
        LARGE_INTEGER ticks;
        QueryPerformanceCounter(&ticks);
        return ticks.QuadPart;
    }
} // namespace orion::platform
