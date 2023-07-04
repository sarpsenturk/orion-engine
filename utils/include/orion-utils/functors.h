#pragma once

namespace orion
{
    struct plus {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs + rhs;
        }
    };

    struct minus {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs - rhs;
        }
    };

    struct multiplies {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs * rhs;
        }
    };

    struct divides {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs / rhs;
        }
    };

    struct modulus {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs % rhs;
        }
    };

    struct negate {
        constexpr auto operator()(auto&& arg) const
        {
            return -arg;
        }
    };

    struct equal_to {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs == rhs;
        }
    };

    struct not_equal_to {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs != rhs;
        }
    };

    struct greater {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs > rhs;
        }
    };

    struct less {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs < rhs;
        }
    };

    struct greater_equal {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs >= rhs;
        }
    };

    struct less_equal {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs <= rhs;
        }
    };

    struct logical_and {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs && rhs;
        }
    };

    struct logical_or {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs || rhs;
        }
    };

    struct logical_not {
        constexpr auto operator()(auto&& arg) const
        {
            return !arg;
        }
    };

    struct bit_and {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs & rhs;
        }
    };

    struct bit_or {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs | rhs;
        }
    };

    struct bit_xor {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs ^ rhs;
        }
    };

    struct bit_not {
        constexpr auto operator()(auto&& arg) const
        {
            return ~arg;
        }
    };
} // namespace orion
