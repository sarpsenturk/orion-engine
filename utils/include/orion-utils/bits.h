#pragma once

#include "assertion.h"

#include <concepts>
#include <limits>

namespace orion
{
    template<std::unsigned_integral T>
    constexpr T get_bit(T value, T nth)
    {
        ORION_ASSERT(nth < std::numeric_limits<T>::digits);
        return value & (T{1} << nth);
    }

    template<std::unsigned_integral T>
    constexpr void set_bit(T& value, T nth)
    {
        ORION_ASSERT(nth < std::numeric_limits<T>::digits);
        value |= (T{1} << nth);
    }

    template<std::unsigned_integral T>
    constexpr void clear_bit(T& value, T nth)
    {
        ORION_ASSERT(nth < std::numeric_limits<T>::digits);
        value &= ~(T{1} << nth);
    }

    template<std::unsigned_integral T>
    constexpr void toggle_bit(T& value, T nth)
    {
        ORION_ASSERT(nth < std::numeric_limits<T>::digits);
        value ^= (T{1} << nth);
    }
} // namespace orion
