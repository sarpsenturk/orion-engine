#include "orion-math/vector/vector.h"
#include "orion-math/vector/vector2.h"
#include "orion-math/vector/vector3.h"
#include "orion-math/vector/vector4.h"

#include <gtest/gtest.h>
#include <iterator>

namespace
{
    TEST(Vector, At)
    {
        constexpr int expected = 421;
        orion::Vector<int, 3> vector{};
        ASSERT_NE(vector.at(0), expected);
        ASSERT_NE(vector.at(1), expected);
        ASSERT_NE(vector.at(2), expected);

        vector.at(0) = expected;
        EXPECT_EQ(vector.at(0), expected);
        vector.at(1) = expected;
        EXPECT_EQ(vector.at(1), expected);
        vector.at(2) = expected;
        EXPECT_EQ(vector.at(2), expected);
    }

    TEST(Vector, AtOutOfRange)
    {
        const orion::Vector vector{1};
        EXPECT_THROW((void)vector.at(1), std::out_of_range);
    }

    TEST(Vector, SubscriptOperator)
    {
        constexpr int expected = 42;
        orion::Vector<int, 3> vector{};
        ASSERT_NE(vector[0], expected);
        ASSERT_NE(vector[1], expected);
        ASSERT_NE(vector[2], expected);

        vector[0] = expected;
        EXPECT_EQ(vector[0], expected);
        vector[1] = expected;
        EXPECT_EQ(vector[1], expected);
        vector[2] = expected;
        EXPECT_EQ(vector[2], expected);
    }

    TEST(Vector, Accesors)
    {
        constexpr int expected = 42;
        auto vector = orion::Vector<int, 4>{};
        ASSERT_NE(vector.x(), expected);
        ASSERT_NE(vector.y(), expected);
        ASSERT_NE(vector.z(), expected);
        ASSERT_NE(vector.w(), expected);

        vector.x() = expected;
        EXPECT_EQ(vector.x(), expected);
        vector.y() = expected;
        EXPECT_EQ(vector.y(), expected);
        vector.z() = expected;
        EXPECT_EQ(vector.z(), expected);
        vector.w() = expected;
        EXPECT_EQ(vector.w(), expected);
    }

    TEST(Vector, SqrMagnitude)
    {
        const auto vector = orion::Vector{1, 2, 3};
        EXPECT_EQ(vector.sqr_magnitude(), 14);
    }

    TEST(Vector, Magnitude)
    {
        const auto vector = orion::Vector{1, 2, 3};
        EXPECT_NEAR(vector.magnitude(), 3.7416573867739413, 1e-8);
    }

    TEST(Vector, Normalized)
    {
        const auto vector = orion::Vector{1, 2, 3};
        const auto expected = orion::Vector{0.2672612419124244, 0.5345224838248488, 0.8017837257372732};
        const auto normalized = vector.normalized();
        EXPECT_NEAR(normalized[0], expected[0], 1e-8);
        EXPECT_NEAR(normalized[1], expected[1], 1e-8);
        EXPECT_NEAR(normalized[2], expected[2], 1e-8);
    }

    TEST(Vector, Normalize)
    {
        auto vector = orion::Vector{1.f, 2.f, 3.f};
        const auto expected = orion::Vector{0.2672612419124244f, 0.5345224838248488f, 0.8017837257372732f};
        vector.normalize();
        EXPECT_EQ(vector, expected);
    }

    TEST(Vector, Front)
    {
        const auto vector = orion::Vector{1, 2, 3};
        EXPECT_EQ(vector.front(), 1);
    }

    TEST(Vector, Back)
    {
        const auto vector = orion::Vector{1, 2, 3};
        EXPECT_EQ(vector.back(), 3);
    }

    TEST(Vector, Data)
    {
        const auto vector = orion::Vector{1, 2, 3};
        const void* ptr = &vector;
        EXPECT_EQ(vector.data(), ptr);
        EXPECT_EQ(*vector.data(), 1);
    }

    TEST(Vector, IsEmptyNonEmptyVector)
    {
        const orion::Vector vector{1};
        EXPECT_FALSE(vector.is_empty());
    }

    TEST(Vector, IsEmptyEmptyVector)
    {
        const orion::Vector<int, 0> vector{};
        EXPECT_TRUE(vector.is_empty());
    }

    TEST(Vector, Size)
    {
        constexpr std::size_t size = 3;
        const orion::Vector<int, size> vector{};
        EXPECT_EQ(vector.size(), size);
    }

    TEST(Vector, MaxSize)
    {
        constexpr std::size_t size = 3;
        const orion::Vector<int, size> vector{};
        EXPECT_EQ(vector.max_size(), size);
    }

    TEST(Vector, ComparisonEqual)
    {
        const orion::Vector vector1{1, 2, 3};
        const orion::Vector vector2{1, 2, 3};
        EXPECT_EQ(vector1, vector2);
    }

    TEST(Vector, ComparisonNotEqual)
    {
        const orion::Vector vector1{1, 2, 3};
        const orion::Vector vector2{4, 5, 6};
        EXPECT_NE(vector1, vector2);
    }

    TEST(Vector, ComparisonEmptyVectors)
    {
        const orion::Vector<int, 0> empty1{};
        const orion::Vector<int, 0> empty2{};
        EXPECT_EQ(empty1, empty2);
    }

    TEST(Vector, ScalarMultiplication)
    {
        const orion::Vector vector{1, 2, 3};
        const auto expected = orion::Vector{3, 6, 9};
        EXPECT_EQ(vector * 3, expected);
    }

    TEST(Vector, ScalarDivision)
    {
        const auto vector = orion::Vector{2, 4, 6};
        const auto expected = orion::Vector{1, 2, 3};
        EXPECT_EQ(vector / 2, expected);
    }

    TEST(Vector, Negation)
    {
        const auto vector = orion::Vector{1, 2, 3};
        const auto expected = orion::Vector{-1, -2, -3};
        EXPECT_EQ(-vector, expected);
    }

    TEST(Vector, VectorAddition)
    {
        const auto lhs = orion::Vector{1, 2, 3};
        const auto rhs = orion::Vector{4, 5, 6};
        const auto expected = orion::Vector{5, 7, 9};
        EXPECT_EQ(lhs + rhs, expected);
    }

    TEST(Vector, VectorSubtraction)
    {
        const auto lhs = orion::Vector{1, 2, 3};
        const auto rhs = orion::Vector{4, 5, 6};
        const auto expected = orion::Vector{-3, -3, -3};
        EXPECT_EQ(lhs - rhs, expected);
    }

    TEST(Vector, PlusEq)
    {
        auto lhs = orion::Vector{1, 2, 3};
        const auto rhs = orion::Vector{4, 5, 6};
        lhs += rhs;
        const auto expected = orion::Vector{5, 7, 9};
        EXPECT_EQ(lhs, expected);
    }

    TEST(Vector, MinusEq)
    {
        auto lhs = orion::Vector{1, 2, 3};
        const auto rhs = orion::Vector{4, 5, 6};
        lhs -= rhs;
        const auto expected = orion::Vector{-3, -3, -3};
        EXPECT_EQ(lhs, expected);
    }

    TEST(Vector, DotProduct)
    {
        const auto lhs = orion::Vector{1, 2, 3};
        const auto rhs = orion::Vector{4, 5, 6};
        const auto expected = 32;
        EXPECT_EQ(orion::dot(lhs, rhs), expected);
    }

    TEST(Vector, CrossProduct)
    {
        const auto lhs = orion::Vector{1, 2, 3};
        const auto rhs = orion::Vector{4, 5, 6};
        const auto expected = orion::Vector{-3, 6, -3};
        EXPECT_EQ(orion::cross(lhs, rhs), expected);
    }

    TEST(Vector, VectorCast)
    {
        const auto vector_f = orion::Vector{1.8f, 2.5f, 3.f};
        const auto vector_i = orion::vector_cast<int>(vector_f);
        const auto expected = orion::Vector{1, 2, 3};
        EXPECT_EQ(vector_i, expected);
    }

    TEST(Vector, Format)
    {
        EXPECT_EQ(fmt::format("{}", orion::Vector{1, 2}), "Vector2(1, 2)");
        EXPECT_EQ(fmt::format("{}", orion::Vector{1, 2, 3}), "Vector3(1, 2, 3)");
        EXPECT_EQ(fmt::format("{}", orion::Vector{1, 2, 3, 4}), "Vector4(1, 2, 3, 4)");
        EXPECT_EQ(fmt::format("{}", orion::Vector{1.5f, 2, 3, 4.1f}), "Vector4(1.5, 2, 3, 4.1)");
    }

    TEST(Vector3, Factory)
    {
        // Vector3 from Vector2
        {
            const auto vec2 = orion::Vector{1.f, 2.f};
            const auto vec3 = orion::vec3(vec2, 3.f);
            const auto expected = orion::Vector{1.f, 2.f, 3.f};
            EXPECT_EQ(vec3, expected);
        }

        // Vector4 from Vector2
        {
            const auto vec2 = orion::Vector{1.f, 2.f};
            const auto vec4 = orion::vec4(vec2, 3.f, 4.f);
            const auto expected = orion::Vector{1.f, 2.f, 3.f, 4.f};
            EXPECT_EQ(vec4, expected);
        }

        // Vector4 from Vector3
        {
            const auto vec3 = orion::Vector{1.f, 2.f, 3.f};
            const auto vec4 = orion::vec4(vec3, 4.f);
            const auto expected = orion::Vector{1.f, 2.f, 3.f, 4.f};
            EXPECT_EQ(vec4, expected);
        }
    }
} // namespace
