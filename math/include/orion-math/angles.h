#pragma once

#include "constants.h"

#include "orion-utils/concepts.h"

#include <concepts>
#include <type_traits>

namespace orion
{
    template<std::floating_point T>
    inline constexpr T to_degrees(T radians) noexcept
    {
        return radians * T{180} / pi_v<T>;
    }

    template<std::floating_point T>
    inline constexpr T to_radians(T degrees) noexcept
    {
        return degrees * pi_v<T> / T{180};
    }

    struct RadianTag;
    struct DegreeTag;

    template<typename From, typename To>
    struct AngleConverter;

    template<typename Tag>
    struct AngleConverter<Tag, Tag> {
        template<std::floating_point T>
        constexpr T operator()(T angle) const noexcept
        {
            return angle;
        }
    };

    template<>
    struct AngleConverter<RadianTag, DegreeTag> {
        template<std::floating_point T>
        constexpr T operator()(T angle) const noexcept
        {
            return to_degrees(angle);
        }
    };

    template<>
    struct AngleConverter<DegreeTag, RadianTag> {
        template<std::floating_point T>
        constexpr T operator()(T angle) const noexcept
        {
            return to_radians(angle);
        }
    };

    template<typename FromTag, typename ToTag, std::floating_point T>
    inline constexpr T convert_angle(T angle) noexcept
    {
        return AngleConverter<FromTag, ToTag>{}(angle);
    }

    template<std::floating_point T, typename Tag>
    class Angle
    {
    public:
        using value_type = T;

        constexpr explicit Angle(value_type value) noexcept
            : value_(value)
        {
        }

        template<std::convertible_to<T> T1, typename TagOther>
        constexpr explicit Angle(const Angle<T1, TagOther>& other) noexcept
            : value_(static_cast<T>(convert_angle<TagOther, Tag>(other.value())))
        {
        }

        template<std::common_with<T> T1, typename TagOther>
        constexpr friend Angle<std::common_type_t<T, T1>, Tag> operator+(Angle lhs, Angle<T1, TagOther> rhs) noexcept
        {
            using common_type = std::common_type_t<T, T1>;
            const auto converted = convert_angle<TagOther, Tag>(rhs.value());
            const auto result = static_cast<common_type>(lhs.value()) + static_cast<common_type>(converted);
            return Angle<common_type, Tag>{result};
        }

        template<std::common_with<T> T1, typename TagOther>
        constexpr friend Angle<std::common_type_t<T, T1>, Tag> operator-(Angle lhs, Angle<T1, TagOther> rhs) noexcept
        {
            using common_type = std::common_type_t<T, T1>;
            const auto converted = convert_angle<TagOther, Tag>(rhs.value());
            const auto result = static_cast<common_type>(lhs.value()) - static_cast<common_type>(converted);
            return Angle<common_type, Tag>{result};
        }

        template<Arithmetic T1>
        constexpr friend Angle operator*(Angle angle, T1 factor) noexcept
        {
            return Angle{angle.value() * factor};
        }

        template<Arithmetic T1>
        constexpr friend Angle operator*(T1 factor, Angle angle) noexcept
        {
            return Angle{factor * angle.value()};
        }

        template<Arithmetic T1>
        constexpr friend Angle operator/(Angle angle, T1 divider) noexcept
        {
            return Angle{angle.value() / divider};
        }

        constexpr friend Angle operator-(Angle angle) noexcept
        {
            return Angle{-angle.value()};
        }

        [[nodiscard]] constexpr value_type value() const noexcept { return value_; }

    private:
        value_type value_;
    };

    template<std::floating_point T>
    using Radian_t = Angle<T, RadianTag>;
    using Radian_d = Radian_t<double>;
    using Radian_f = Radian_t<float>;

    template<std::floating_point T>
    using Degree_t = Angle<T, DegreeTag>;
    using Degree_d = Degree_t<double>;
    using Degree_f = Degree_t<float>;

    template<std::floating_point T>
    inline constexpr Radian_t<T> radians(T value) noexcept
    {
        return Radian_t<T>{value};
    }

    template<std::floating_point T>
    inline constexpr Degree_t<T> degrees(T value) noexcept
    {
        return Degree_t<T>{value};
    }

    template<std::floating_point T>
    inline constexpr Radian_t<T> to_radians(Degree_t<T> degrees) noexcept
    {
        return Radian_t<T>{degrees};
    }

    template<std::floating_point T>
    inline constexpr Degree_t<T> to_degrees(Radian_t<T> radians) noexcept
    {
        return Degree_t<T>{radians};
    }

    inline constexpr auto pi_radians = radians(pi);

    inline namespace literals
    {
        inline namespace angle_literals
        {
            inline constexpr auto operator""_rad(long double value) noexcept
            {
                return radians(value);
            }

            inline constexpr auto operator""_deg(long double value) noexcept
            {
                return degrees(value);
            }
        } // namespace angle_literals
    } // namespace literals
} // namespace orion
