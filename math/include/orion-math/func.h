#pragma once

namespace orion::math
{
    template<typename T = void>
    struct plus;

    template<>
    struct plus<void> {
        [[nodiscard]] constexpr auto operator()(auto&& lhs, auto&& rhs) const
            noexcept(noexcept(lhs + rhs)) -> decltype(lhs + rhs)
        {
            return lhs + rhs;
        }
    };

    template<typename T = void>
    struct minus;

    template<>
    struct minus<void> {
        [[nodiscard]] constexpr auto operator()(auto&& lhs, auto&& rhs) const
            noexcept(noexcept(lhs - rhs)) -> decltype(lhs - rhs)
        {
            return lhs - rhs;
        }
    };

    template<typename T = void>
    struct multiplies;

    template<>
    struct multiplies<void> {
        [[nodiscard]] constexpr auto operator()(auto&& lhs, auto&& rhs) const
            noexcept(noexcept(lhs* rhs)) -> decltype(lhs * rhs)
        {
            return lhs * rhs;
        }
    };

    template<typename T = void>
    struct divides;

    template<>
    struct divides<void> {
        [[nodiscard]] constexpr auto operator()(auto&& lhs, auto&& rhs) const
            noexcept(noexcept(lhs / rhs)) -> decltype(lhs / rhs)
        {
            return lhs / rhs;
        }
    };

    template<typename T = void>
    struct modulus;

    template<>
    struct modulus<void> {
        [[nodiscard]] constexpr auto operator()(auto&& lhs, auto&& rhs) const
            noexcept(noexcept(lhs % rhs)) -> decltype(lhs % rhs)
        {
            return lhs % rhs;
        }
    };

    template<typename T = void>
    struct negate;

    template<>
    struct negate<void> {
        [[nodiscard]] constexpr auto operator()(auto&& arg) const
            noexcept(noexcept(-arg)) -> decltype(-arg)
        {
            return -arg;
        }
    };
} // namespace orion::math
