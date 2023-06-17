#pragma once

#include "angles.h"
#include "constants.h"

#include <cmath> // runtime implementations

namespace orion::math
{
    namespace detail
    {
        template<std::floating_point Return = double>
        [[nodiscard]] constexpr Return sin_taylor_series(Radians radians) noexcept
        {
            auto mod = [](double x, double y) -> double { return x - static_cast<int>(x / y) * y; };
            auto x = mod(radians.value(), 2 * pi);
            char sign = 1;
            if (x > pi) {
                x -= pi;
                sign = -1;
            } else if (x < -pi) {
                x += pi;
                sign = -1;
            }

            const auto x_2 = x * x;
            const auto x_3 = x * x * x;
            const auto x_5 = x_3 * x_2;
            const auto x_7 = x_5 * x_2;
            const auto x_9 = x_7 * x_2;
            const auto x_11 = x_9 * x_2;
            const auto x_13 = x_11 * x_2;
            const auto x_15 = x_13 * x_2;
            constexpr auto fact_3 = 6;
            constexpr auto fact_5 = 120;
            constexpr auto fact_7 = 5'040;
            constexpr auto fact_9 = 362'880;
            constexpr auto fact_11 = 39'916'800;
            constexpr auto fact_13 = 6'227'020'800;
            constexpr auto fact_15 = 1'307'674'368'000;

            return static_cast<Return>(
                sign *
                (x -
                 (x_3 / fact_3) +
                 (x_5 / fact_5) -
                 (x_7 / fact_7) +
                 (x_9 / fact_9) -
                 (x_11 / fact_11) +
                 (x_13 / fact_13) -
                 (x_15 / fact_15)));
        }

        template<std::floating_point Return = double>
        [[nodiscard]] constexpr Return cos_taylor_series(Radians radians) noexcept
        {
            auto mod = [](double x, double y) -> double { return x - static_cast<int>(x / y) * y; };
            auto x = mod(radians.value(), 2 * pi);
            char sign = 1;
            if (x > pi) {
                x -= pi;
                sign = -1;
            } else if (x < -pi) {
                x += pi;
                sign = -1;
            }

            const auto x_2 = x * x;
            const auto x_4 = x_2 * x_2;
            const auto x_6 = x_4 * x_2;
            const auto x_8 = x_6 * x_2;
            const auto x_10 = x_8 * x_2;
            const auto x_12 = x_10 * x_2;
            const auto x_14 = x_12 * x_2;
            constexpr auto fact_2 = 2;
            constexpr auto fact_4 = 24;
            constexpr auto fact_6 = 720;
            constexpr auto fact_8 = 40'320;
            constexpr auto fact_10 = 3'628'800;
            constexpr auto fact_12 = 479'001'600;
            constexpr auto fact_14 = 87'178'291'200;

            return static_cast<Return>(
                sign *
                (1 -
                 (x_2 / fact_2) +
                 (x_4 / fact_4) -
                 (x_6 / fact_6) +
                 (x_8 / fact_8) -
                 (x_10 / fact_10) +
                 (x_12 / fact_12) -
                 (x_14 / fact_14)));
        }
    } // namespace detail

    template<std::floating_point Return = double>
    [[nodiscard]] constexpr Return sin(Radians radians) noexcept
    {
        if (std::is_constant_evaluated()) {
            return detail::sin_taylor_series<Return>(radians);
        }
        return static_cast<Return>(std::sin(radians.value()));
    }

    template<std::floating_point Return = double>
    [[nodiscard]] constexpr Return cos(Radians radians) noexcept
    {
        if (std::is_constant_evaluated()) {
            return detail::cos_taylor_series<Return>(radians);
        }
        return static_cast<Return>(std::cos(radians.value()));
    }

    template<std::floating_point Return = double>
    [[nodiscard]] constexpr Return tan(Radians radians) noexcept
    {
        return sin<Return>(radians) / cos<Return>(radians);
    }

    template<std::floating_point Return = double>
    [[nodiscard]] constexpr Return cot(Radians radians) noexcept
    {
        return cos<Return>(radians) / sin<Return>(radians);
    }
} // namespace orion::math
