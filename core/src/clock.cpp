#include "orion-core/clock.h"

namespace orion::detail
{
    QPCClock::time_point QPCClock::now()
    {
        static const auto freq = platform::query_performance_frequency();
        const auto ticks = platform::query_performance_counter();
        return time_point(duration(static_cast<rep>(ticks * period::den / freq)));
    }
} // namespace orion::detail
