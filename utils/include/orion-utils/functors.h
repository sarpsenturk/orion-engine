#pragma once

namespace orion
{
    struct Plus {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs + rhs;
        }
    };

    struct Minus {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs - rhs;
        }
    };

    struct Multiplies {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs * rhs;
        }
    };

    struct Divides {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs / rhs;
        }
    };

    struct Modulus {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs % rhs;
        }
    };

    struct Negate {
        constexpr auto operator()(auto&& arg) const
        {
            return -arg;
        }
    };

    struct EqualTo {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs == rhs;
        }
    };

    struct NotEqualTo {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs != rhs;
        }
    };

    struct Greater {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs > rhs;
        }
    };

    struct Less {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs < rhs;
        }
    };

    struct GreaterEqual {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs >= rhs;
        }
    };

    struct LessEqual {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs <= rhs;
        }
    };

    struct LogicalAnd {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs && rhs;
        }
    };

    struct LogicalOr {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs || rhs;
        }
    };

    struct LogicalNot {
        constexpr auto operator()(auto&& arg) const
        {
            return !arg;
        }
    };

    struct BitAnd {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs & rhs;
        }
    };

    struct BitOr {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs | rhs;
        }
    };

    struct BitXor {
        constexpr auto operator()(auto&& lhs, auto&& rhs) const
        {
            return lhs ^ rhs;
        }
    };

    struct BitNot {
        constexpr auto operator()(auto&& arg) const
        {
            return ~arg;
        }
    };
} // namespace orion
