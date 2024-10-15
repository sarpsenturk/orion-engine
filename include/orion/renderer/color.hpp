#pragma once

#include "orion/math/vector/vector4.hpp"

#include <cstdint>

namespace orion
{
    using Color = Vector4f;

    // Normalize 8-bit value [0, 255] -> [0, 1]
    constexpr float rgb_normalize(std::uint8_t value)
    {
        return static_cast<float>(value) / 255.0f;
    }

    constexpr Color color_rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255)
    {
        return {
            rgb_normalize(r),
            rgb_normalize(g),
            rgb_normalize(b),
            rgb_normalize(a),
        };
    }

    constexpr Color color_hex(std::uint32_t hex)
    {
        const auto r = rgb_normalize(static_cast<std::uint8_t>((hex & 0x00ff0000) >> 16u));
        const auto g = rgb_normalize(static_cast<std::uint8_t>((hex & 0x0000ff00) >> 8u));
        const auto b = rgb_normalize(static_cast<std::uint8_t>(hex & 0x000000ff));
        return {r, g, b, 1.f};
    }

    namespace colors
    {
        inline constexpr auto red = color_rgba(255, 0, 0);
        inline constexpr auto green = color_rgba(0, 255, 0);
        inline constexpr auto blue = color_rgba(0, 0, 255);
        inline constexpr auto yellow = color_rgba(255, 255, 0);
        inline constexpr auto white = color_rgba(255, 255, 255);
        inline constexpr auto black = color_rgba(0, 0, 0);
        inline constexpr auto magenta = color_rgba(255, 0, 255);
    } // namespace colors
} // namespace orion
