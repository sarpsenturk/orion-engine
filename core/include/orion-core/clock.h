#pragma once

#include "orion-platform/clock.h"

namespace orion
{
    using Clock = platform::HighResolutionClock;
    using FrameTime = typename Clock::duration;
} // namespace orion
