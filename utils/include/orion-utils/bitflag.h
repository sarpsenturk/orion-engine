#pragma once

#include "type.h"

#include <limits>
#include <numeric>
#include <type_traits>

namespace orion
{
    template<typename Enum>
    class Bitflag
    {
    public:
        using enum_type = Enum;
        using underlying_type = std::underlying_type_t<enum_type>;
        static_assert(std::is_unsigned_v<underlying_type>);

        static constexpr auto zero = underlying_type{0};
        static constexpr auto max = std::numeric_limits<underlying_type>::max();

        // Constructors

        constexpr Bitflag() = default;
        constexpr Bitflag(enum_type set_bit)
            : value_(enum_to_value(set_bit))
        {
        }
        constexpr explicit Bitflag(underlying_type value)
            : value_(value)
        {
        }

        // Factory functions
        static constexpr Bitflag all() noexcept { return Bitflag{max}; }
        static constexpr Bitflag none() noexcept { return Bitflag{zero}; }
        static constexpr Bitflag disjunction(std::initializer_list<enum_type> values) noexcept
        {
            auto bit_or = [](Bitflag lhs, Bitflag rhs) { return lhs | rhs; };
            return std::accumulate(values.begin(), values.end(), Bitflag::none(), bit_or);
        }
        static constexpr Bitflag conjunction(std::initializer_list<enum_type> values) noexcept
        {
            auto bit_and = [](Bitflag lhs, Bitflag rhs) { return lhs & rhs; };
            return std::accumulate(values.begin(), values.end(), Bitflag::all(), bit_and);
        }

        // Accessors
        constexpr underlying_type value() const noexcept { return value_; }

        constexpr bool has_all() const noexcept { return value_ == max; }
        constexpr bool has_any() const noexcept { return value_ != zero; }
        constexpr bool has_none() const noexcept { return value_ == zero; }
        constexpr bool has(enum_type bit) const noexcept { return (value_ & enum_to_value(bit)) != 0; }

        // Equality operators
        constexpr bool operator==(const Bitflag& rhs) const noexcept = default;

        // Operator overloads
        constexpr Bitflag operator~() const noexcept
        {
            return Bitflag{static_cast<underlying_type>(~value_)};
        }

        constexpr Bitflag operator|(Bitflag rhs) const noexcept
        {
            return Bitflag{static_cast<underlying_type>(value_ | rhs.value_)};
        }
        constexpr Bitflag& operator|=(Bitflag rhs) noexcept
        {
            *this = (*this | rhs);
            return *this;
        }

        constexpr Bitflag operator&(Bitflag rhs) const noexcept
        {
            return Bitflag{static_cast<underlying_type>(value_ & rhs.value_)};
        }
        constexpr Bitflag& operator&=(Bitflag rhs) noexcept
        {
            *this = (*this & rhs);
            return *this;
        }

        constexpr Bitflag operator^(Bitflag rhs) const noexcept
        {
            return Bitflag{static_cast<underlying_type>(value_ ^ rhs.value_)};
        }
        constexpr Bitflag& operator^=(Bitflag rhs) noexcept
        {
            *this = (*this ^ rhs);
            return *this;
        }

        constexpr Bitflag operator<<(std::size_t shift) const noexcept
        {
            return Bitflag{static_cast<underlying_type>(value_ << shift)};
        }
        constexpr Bitflag& operator<<=(std::size_t shift) noexcept
        {
            *this = (*this << shift);
            return *this;
        }

        constexpr Bitflag operator>>(std::size_t shift) const noexcept
        {
            return Bitflag{static_cast<underlying_type>(value_ >> shift)};
        }
        constexpr Bitflag& operator>>=(std::size_t shift) noexcept
        {
            *this = (*this >> shift);
            return *this;
        }

    private:
        static constexpr auto enum_to_value(enum_type bit) { return static_cast<underlying_type>(1u << to_underlying(bit)); }

        underlying_type value_{};
    };
} // namespace orion
