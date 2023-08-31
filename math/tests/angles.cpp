#include "orion-math/angles.h"

#include <gtest/gtest.h>

namespace
{
    constexpr auto acceptable_error = 1e-5;

    TEST(Angle, Conversion)
    {
        // Radians -> Degrees
        {
            const auto radians = orion::radians(orion::pi);
            const auto degrees = orion::to_degrees(radians);
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(degrees)>, orion::Degree_t<double>>);
            EXPECT_NEAR(degrees.value(), 180.0, acceptable_error);
        }

        // Degrees -> Radians
        {
            const auto degrees = orion::degrees(90.0);
            const auto radians = orion::to_radians(degrees);
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(radians)>, orion::Radian_t<double>>);
            EXPECT_NEAR(radians.value(), 1.57079633, acceptable_error);
        }
    }

    TEST(Angle, Addition)
    {
        // Radians + Radians
        {
            const auto lhs = orion::radians(1.5);
            const auto rhs = orion::radians(2.0);
            const auto sum = lhs + rhs;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(sum)>, orion::Radian_t<double>>);
            EXPECT_EQ(sum.value(), 3.5);
        }

        // Radians + Degrees
        {
            const auto lhs = orion::radians(1.5);
            const auto rhs = orion::degrees(45.0);
            const auto sum = lhs + rhs;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(sum)>, orion::Radian_t<double>>);
            EXPECT_NEAR(sum.value(), 2.28539816, acceptable_error);
        }

        // Degrees + Degrees
        {
            const auto lhs = orion::degrees(45.0);
            const auto rhs = orion::degrees(90.0);
            const auto sum = lhs + rhs;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(sum)>, orion::Degree_t<double>>);
            EXPECT_EQ(sum.value(), 135.0);
        }

        // Degrees + Radians
        {
            const auto lhs = orion::degrees(45.f);
            const auto rhs = orion::radians(1.5);
            const auto sum = lhs + rhs;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(sum)>, orion::Degree_t<double>>);
            EXPECT_NEAR(sum.value(), 130.943669, acceptable_error);
        }
    }

    TEST(Angle, Subtraction)
    {
        // Radians - Radians
        {
            const auto lhs = orion::radians(1.5);
            const auto rhs = orion::radians(2.0);
            const auto sum = lhs - rhs;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(sum)>, orion::Radian_t<double>>);
            EXPECT_EQ(sum.value(), -0.5);
        }

        // Radians - Degrees
        {
            const auto lhs = orion::radians(1.5);
            const auto rhs = orion::degrees(45.0);
            const auto sum = lhs - rhs;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(sum)>, orion::Radian_t<double>>);
            EXPECT_NEAR(sum.value(), 0.714601837, acceptable_error);
        }

        // Degrees  Degrees
        {
            const auto lhs = orion::degrees(45.0);
            const auto rhs = orion::degrees(90.0);
            const auto sum = lhs - rhs;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(sum)>, orion::Degree_t<double>>);
            EXPECT_EQ(sum.value(), -45.0);
        }

        // Degrees - Radians
        {
            const auto lhs = orion::degrees(45.f);
            const auto rhs = orion::radians(1.5);
            const auto sum = lhs - rhs;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(sum)>, orion::Degree_t<double>>);
            EXPECT_NEAR(sum.value(), -40.9436693, acceptable_error);
        }
    }

    TEST(Angle, Multiplication)
    {
        const auto radians = orion::radians(1.5);
        const auto factor = 2.0;

        // Radian * factor
        {
            const auto result = radians * factor;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(result)>, orion::Radian_t<double>>);
            EXPECT_EQ(result.value(), 3.0);
        }

        // Factor * Radian
        {
            const auto result = factor * radians;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(result)>, orion::Radian_t<double>>);
            EXPECT_EQ(result.value(), 3.0);
        }

        const auto degrees = orion::degrees(90.0);
        // Degree * factor
        {
            const auto result = degrees * factor;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(result)>, orion::Degree_t<double>>);
            EXPECT_EQ(result.value(), 180.0);
        }

        // Factor * degree
        {
            const auto result = factor * degrees;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(result)>, orion::Degree_t<double>>);
            EXPECT_EQ(result.value(), 180.0);
        }
    }

    TEST(Angle, Division)
    {
        // Radian division
        {
            const auto radians = orion::radians(orion::pi);
            const auto factor = 2.0;
            const auto result = radians / factor;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(result)>, orion::Radian_t<double>>);
            EXPECT_NEAR(result.value(), orion::pi / 2.0, acceptable_error);
        }

        // Degree division
        {
            const auto degrees = orion::degrees(90.0);
            const auto factor = 2.0;
            const auto result = degrees / factor;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(result)>, orion::Degree_t<double>>);
            EXPECT_NEAR(result.value(), 45.0, acceptable_error);
        }
    }

    TEST(Angle, Negation)
    {
        // Radians
        {
            const auto radians = orion::radians(1.5);
            const auto negated = -radians;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(negated)>, orion::Radian_t<double>>);
            EXPECT_EQ(negated.value(), -1.5);
        }

        // Degrees
        {
            const auto degrees = orion::degrees(135.0);
            const auto negated = -degrees;
            static_assert(std::is_same_v<std::remove_cvref_t<decltype(negated)>, orion::Degree_t<double>>);
            EXPECT_EQ(negated.value(), -135.0);
        }
    }
} // namespace
