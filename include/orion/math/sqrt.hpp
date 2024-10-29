#pragma once

#include "orion/assertion.hpp"

#include <cmath>
#include <concepts>
#include <type_traits>

namespace orion::math
{
    namespace detail
    {
        template<std::floating_point T>
        constexpr T sqrt_impl(T value)
        {
            if (value < T{-0.0}) {
                return NAN;
            }

            if (value == T{+0.0} || value == T{-0.0} || value == T{+INFINITY}) {
                return value;
            }

            constexpr int iterations = 5;
            T guess = value / 2;
            for (int i = 0; i <= iterations; ++i) {
                guess = (guess + value / guess) / 2;
            }
            return guess;
        }
    } // namespace detail

    template<std::floating_point T>
    constexpr T sqrt(T value)
    {
#ifdef ORION_FORCE_SQRT_IMPL
        return detail::sqrt_impl(value);
#else
        if (std::is_constant_evaluated()) {
            return detail::sqrt_impl(value);
        } else {
            return std::sqrt(value);
        }
#endif
    }

    template<std::integral T>
    constexpr double sqrt(T value)
    {
        return sqrt(static_cast<double>(value));
    }
} // namespace orion::math
