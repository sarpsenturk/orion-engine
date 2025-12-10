#pragma once

#include <numbers>

namespace orion
{
    template<typename T>
    constexpr T radians(T degrees)
    {
        return degrees * std::numbers::pi_v<T> / T{180};
    }

    template<typename T>
    constexpr T degrees(T radians)
    {
        return radians * T{180} / std::numbers::pi_v<T>;
    }
} // namespace orion