#pragma once

#include <concepts>

namespace orion
{
    template<std::floating_point Floating>
    [[nodiscard]] constexpr Floating abs(Floating value) noexcept
    {
        if (value > 0) {
            return value;
        }
        return -value;
    }

    template<std::integral Integral>
    [[nodiscard]] double abs(Integral value) noexcept
    {
        return abs(static_cast<double>(value));
    }
} // namespace orion
