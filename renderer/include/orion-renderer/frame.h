#pragma once

#include "orion-utils/array.h"

namespace orion
{
    using frame_index_t = std::int32_t;

    inline constexpr auto frames_in_flight = ORION_FRAMES_IN_FLIGHT;
    static_assert(frames_in_flight > 0);

    template<typename T>
    using PerFrame = std::array<T, frames_in_flight>;

    auto generate_per_frame(auto&& value)
    {
        return make_array<frames_in_flight>(std::forward<decltype(value)>(value));
    }
} // namespace orion
