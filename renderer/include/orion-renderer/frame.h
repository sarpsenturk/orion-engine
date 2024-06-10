#pragma once

#include <array>
#include <concepts>
#include <cstdint>
#include <functional>
#include <type_traits>
#include <utility>

namespace orion
{
    using frame_index_t = std::int8_t;

    inline constexpr auto frames_in_flight = ORION_FRAMES_IN_FLIGHT;
    static_assert(frames_in_flight > 0);

    template<typename T>
    using PerFrame = std::array<T, frames_in_flight>;

    template<std::invocable<frame_index_t> Generator, frame_index_t... FrameIndex>
    auto generate_per_frame_impl(const Generator& generator, std::integer_sequence<frame_index_t, FrameIndex...>) -> PerFrame<std::invoke_result_t<Generator, frame_index_t>>
    {
        return {std::invoke(generator, FrameIndex)...};
    }

    template<std::invocable<frame_index_t> Generator>
    auto generate_per_frame(const Generator& generator) -> PerFrame<std::invoke_result_t<Generator, frame_index_t>>
    {
        return generate_per_frame_impl(generator, std::make_integer_sequence<frame_index_t, frames_in_flight>{});
    }
} // namespace orion
