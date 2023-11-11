// This file contains utilities to use enumerations (especially scoped enums)
// as bitflags / bitmasks.
//
// Bitwise operators:
// 2 ways to enable bitwise operations on your enums are:
//  - Have the ORION_ENABLE_BITWISE value in your enum
//      ```
//      enum class MyEnum {
//          First,
//          Second,
//          ORION_ENABLE_BITWISE
//      };
//      ```
//  - Specialize orion::enum_bitwise_enabled type trait
//      ```
//      enum class MyEnum {
//          First,
//          Second,
//      };
//      template<>
//      struct orion::enum_bitwise_enabled<MyEnum> : std::true_type {};
//      ```
// Bitwise operators for enums satisfying either of those
// are defined in global scope which allows it to be easily used while
// keeping default behaviour for any other enum types.
//
// Bitwise range
// We also provide BitwiseRange which allows for iterating over
// bits of a bitwise_enum type.
// As of not BitwiseRange does not skip unset bits. It will loop
// over every bit of the enum, number of which depends on the
// underlying type.
//
// To specifically loop over set bits, use the for_set_bit()
// function

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
    concept bitwise_enum = requires() { E::ORION_ENABLE_BITWISE; } || enum_bitwise_enabled_v<E>;

    template<bitwise_enum E, typename F>
    void for_set_bit(E value, F&& callback)
    {
        const auto bit_count = std::numeric_limits<std::underlying_type_t<E>>::digits;
        for (std::size_t i = 0; i < bit_count; ++i) {
            if (auto value_at = to_underlying(value) & (1ull << i)) {
                callback(static_cast<E>(value_at));
            }
        }
    }

    template<bitwise_enum E>
    class BitwiseRange
    {
    public:
        static constexpr auto bit_count = std::numeric_limits<std::underlying_type_t<E>>::digits;

        class Iterator
        {
        public:
            using difference_type = std::size_t;
            using value_type = E;

            constexpr Iterator() = default;
            constexpr Iterator(value_type value, difference_type index)
                : value_(value)
                , index_(index)
            {
                ORION_ASSERT(index >= 0 && index <= bit_count);
            }

            constexpr Iterator& operator++() noexcept
            {
                ORION_ASSERT(index_ < bit_count);
                ++index_;
                return *this;
            }
            constexpr Iterator operator++(int) noexcept
            {
                ORION_ASSERT(index_ < bit_count);
                auto temp = *this;
                ++index_;
                return temp;
            }

            constexpr Iterator& operator--() noexcept
            {
                ORION_ASSERT(index_ > 0);
                --index_;
                return *this;
            }
            constexpr Iterator operator--(int) noexcept
            {
                ORION_ASSERT(index_ > 0);
                auto temp = *this;
                --index_;
                return temp;
            }

            constexpr value_type operator*() noexcept { return value_at_index(); }
            constexpr value_type operator*() const noexcept { return value_at_index(); }

            constexpr bool operator==(const Iterator&) const noexcept = default;

        private:
            constexpr auto value_at_index() const noexcept
            {
                ORION_ASSERT(index_ >= 0 && index_ < bit_count);
                return static_cast<value_type>(to_underlying(value_) & (1ull << index_));
            }

            value_type value_;
            difference_type index_;
        };

        using iterator = Iterator;

        constexpr explicit BitwiseRange(E value)
            : value_(value)
        {
        }

        constexpr iterator begin() noexcept { return Iterator{value_, 0}; }
        constexpr iterator begin() const noexcept { return Iterator{value_, 0}; }
        constexpr iterator end() noexcept { return Iterator{value_, bit_count}; }
        constexpr iterator end() const noexcept { return Iterator{value_, bit_count}; }

    private:
        E value_;
    };
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
