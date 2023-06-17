#pragma once

#include "abs.h"

#include <concepts>
#include <limits>

namespace orion::math
{
    template<std::floating_point Floating>
    [[nodiscard]] constexpr Floating sqrt(Floating value) noexcept
    {
        // TODO: Add precondition for negative values

        if (value == Floating{0}) {
            return Floating{0};
        }

        auto result = value / 2;
        while (true) {
            auto next = Floating{0.5} * (result + value / result);
            auto diff = abs(result - next);
            if (diff <= std::numeric_limits<Floating>::epsilon()) {
                break;
            }
            result = next;
        }
        return result;
    }

    template<std::integral Integral>
    [[nodiscard]] constexpr double sqrt(Integral value) noexcept
    {
        return sqrt(static_cast<double>(value));
    }
} // namespace orion::math
