#pragma once

#include "abs.h"
#include "concepts.h"
#include "constants.h" // pi

#include <ratio> // std::ratio, std::ratio_divide

namespace orion::math
{
    template<typename Ratio = std::ratio<1>>
    class Angle;

    template<typename To, typename Ratio>
    [[nodiscard]] constexpr To angle_cast(const Angle<Ratio>& angle);

    template<typename Ratio>
    class Angle
    {
    public:
        using rep = double;
        using ratio = Ratio;

        constexpr Angle() = default;

        constexpr explicit Angle(rep value)
            : value_(value)
        {
        }

        template<typename Rep1>
        constexpr explicit Angle(Rep1 value)
            : value_(static_cast<rep>(value))
        {
        }

        template<typename Ratio1>
        constexpr Angle(const Angle<Ratio1>& other)
            : value_(angle_cast<Angle>(other).value())
        {
        }

        [[nodiscard]] constexpr auto value() const noexcept { return value_; }

        template<typename Ratio1>
        constexpr friend bool operator==(const Angle& lhs, const Angle<Ratio1>& rhs) noexcept
        {
            return orion::math::abs(lhs.value_ - angle_cast<Angle>(rhs).value_) <= acceptable_error;
        }

        template<typename Ratio1>
        constexpr friend Angle operator+(const Angle& lhs, const Angle<Ratio1>& rhs) noexcept
        {
            return Angle{lhs.value_ + angle_cast<Angle>(rhs).value_};
        }
        template<typename Ratio1>
        constexpr friend Angle operator-(const Angle& lhs, const Angle<Ratio1>& rhs) noexcept
        {
            return Angle{lhs.value_ - angle_cast<Angle>(rhs).value_};
        }

        template<arithmetic T>
        constexpr friend Angle operator*(const Angle& angle, T value) noexcept(noexcept(angle.value() * value))
        {
            return Angle{angle.value_ * value};
        }
        template<arithmetic T>
        constexpr friend Angle operator*(T value, const Angle& angle) noexcept(noexcept(angle.value() * value))
        {
            return Angle{angle.value() * value};
        }

        template<arithmetic T>
        constexpr friend Angle operator/(const Angle& angle, T value) noexcept(noexcept(angle.value() / value))
        {
            return Angle{angle.value_ / value};
        }
        template<arithmetic T>
        constexpr friend Angle operator/(T value, const Angle& angle) noexcept(noexcept(angle.value() / value))
        {
            return Angle{angle.value_ / value};
        }

        constexpr Angle operator-() const noexcept
        {
            return Angle{-value_};
        }

    private:
        static constexpr auto acceptable_error = 1e-5;

        rep value_;
    };

    template<typename To, typename Ratio>
    [[nodiscard]] constexpr To angle_cast(const Angle<Ratio>& angle)
    {
        using common_ratio = std::ratio_divide<Ratio, typename To::ratio>;
        return To{angle.value() * common_ratio::num / common_ratio::den};
    }

    using Radians = Angle<>;
    using Degrees = Angle<std::ratio<17'453'293, 1'000'000'000>>;

    inline constexpr Radians pi_rads{pi};

    namespace angle_literals
    {
        [[nodiscard]] constexpr Radians operator"" _rad(long double value) { return Radians{value}; }
        [[nodiscard]] constexpr Radians operator"" _rad(std::uint64_t value) { return Radians{value}; }
        [[nodiscard]] constexpr Degrees operator"" _deg(long double value) { return Degrees{value}; }
        [[nodiscard]] constexpr Degrees operator"" _deg(std::uint64_t value) { return Degrees{value}; }
    } // namespace angle_literals
} // namespace orion::math
