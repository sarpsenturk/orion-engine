// This file contains utilities to use enumerations (especially scoped enums)
// as bitflags / bitmasks.

#pragma once

#include "type.h"

#include "orion-utils/assertion.h"

#include <compare>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace orion
{
    template<typename E>
    struct enum_bitwise_enabled : std::false_type {
    };

    template<typename E>
    inline constexpr auto enum_bitwise_enabled_v = enum_bitwise_enabled<E>::value;

    template<typename E>
    concept bitwise_enum = enum_bitwise_enabled_v<E>;
} // namespace orion

template<orion::bitwise_enum E>
constexpr E operator~(E value)
{
    return static_cast<E>(~orion::to_underlying(value));
}

template<orion::bitwise_enum E>
constexpr E operator|(E lhs, E rhs)
{
    return static_cast<E>(orion::to_underlying(lhs) | orion::to_underlying(rhs));
}

template<orion::bitwise_enum E>
constexpr E& operator|=(E& lhs, E rhs)
{
    lhs = lhs | rhs;
    return lhs;
}

template<orion::bitwise_enum E>
constexpr E operator&(E lhs, E rhs)
{
    return static_cast<E>(orion::to_underlying(lhs) & orion::to_underlying(rhs));
}
template<orion::bitwise_enum E>
constexpr E& operator&=(E& lhs, E rhs)
{
    lhs = lhs & rhs;
    return lhs;
}

template<orion::bitwise_enum E>
constexpr E operator^(E lhs, E rhs)
{
    return static_cast<E>(orion::to_underlying(lhs) ^ orion::to_underlying(rhs));
}
template<orion::bitwise_enum E>
constexpr E& operator^=(E& lhs, E rhs)
{
    lhs = lhs ^ rhs;
    return lhs;
}

template<orion::bitwise_enum E>
constexpr E operator<<(E value, std::size_t shift)
{
    return static_cast<E>(orion::to_underlying(value) << shift);
}
template<orion::bitwise_enum E>
constexpr E& operator<<=(E& value, std::size_t shift)
{
    value = value << shift;
    return value;
}

template<orion::bitwise_enum E>
constexpr E operator>>(E value, std::size_t shift)
{
    return static_cast<E>(orion::to_underlying(value) >> shift);
}
template<orion::bitwise_enum E>
constexpr E& operator>>=(E& value, std::size_t shift)
{
    value = value >> shift;
    return value;
}

template<orion::bitwise_enum E>
constexpr bool operator!(E value)
{
    return orion::to_underlying(value) == 0;
}

#define ORION_BITFLAG(name, base)                                 \
    enum class name : base;                                       \
    template<>                                                    \
    struct ::orion::enum_bitwise_enabled<name> : std::true_type { \
    };                                                            \
    enum class name : base
