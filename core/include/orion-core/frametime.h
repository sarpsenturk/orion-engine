#pragma once

#include "orion-core/clock.h"

#include "orion-math/vector/vector.h"

namespace orion
{
    using FrameTime = typename Clock::duration;

    template<typename T, std::size_t N>
    constexpr Vector<T, N> operator*(const Vector<T, N>& vector, const FrameTime& frame_time)
    {
        return vector * frame_time.count();
    }
} // namespace orion
