#pragma once

#include "orion-math/vector/vector4.h"

#include <cstdint>

namespace orion
{
    using Color = Vector4_f;

    constexpr auto rgba_normalize(std::uint8_t value) -> float
    {
        return static_cast<float>(value) / 255;
    }

    constexpr auto color_hex(std::uint32_t value) -> Color
    {
        const auto r = rgba_normalize((value & 0x00ff0000) >> 16u);
        const auto g = rgba_normalize((value & 0x0000ff00) >> 8u);
        const auto b = rgba_normalize(value & 0x000000ff);
        return {r, g, b, 1.f};
    }

    constexpr auto color_rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b) -> Color
    {
        return {rgba_normalize(r), rgba_normalize(g), rgba_normalize(b), 1.f};
    }

    constexpr auto color_rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a) -> Color
    {
        return {rgba_normalize(r), rgba_normalize(g), rgba_normalize(b), rgba_normalize(a)};
    }

    namespace colors
    {
        inline constexpr auto black = color_hex(0x000000);
        inline constexpr auto white = color_hex(0xFFFFFF);
        inline constexpr auto red = color_hex(0xFF0000);
        inline constexpr auto lime = color_hex(0x00FF00);
        inline constexpr auto blue = color_hex(0x0000FF);
        inline constexpr auto yellow = color_hex(0xFFFF00);
        inline constexpr auto cyan = color_hex(0x00FFFF);
        inline constexpr auto magenta = color_hex(0xFF00FF);
        inline constexpr auto silver = color_hex(0xC0C0C0);
        inline constexpr auto gray = color_hex(0x808080);
        inline constexpr auto maroon = color_hex(0x800000);
        inline constexpr auto olive = color_hex(0x808000);
        inline constexpr auto green = color_hex(0x008000);
        inline constexpr auto purple = color_hex(0x800080);
        inline constexpr auto teal = color_hex(0x008080);
        inline constexpr auto navy = color_hex(0x000080);
    } // namespace colors
} // namespace orion
