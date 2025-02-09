#include "orion/math/vector/vector.hpp"
#include "orion/math/vector/vector3.hpp"

#include <gtest/gtest.h>

namespace orion
{
    namespace
    {
        TEST(Vector, Size)
        {
            const auto vector = Vector{1, 2, 3};
            ASSERT_EQ(vector.size(), 3);
        }

        TEST(Vector, SizeEmpty)
        {
            const auto vector = Vector<int, 0>{};
            ASSERT_EQ(vector.size(), 0);
        }

        TEST(Vector, Empty)
        {
            const auto vector = Vector<int, 0>{};
            ASSERT_TRUE(vector.empty());
        }

        TEST(Vector, NonEmpty)
        {
            const auto vector = Vector<int, 1>{};
            ASSERT_FALSE(vector.empty());
        }

        TEST(Vector, Subscript)
        {
            const auto vector = Vector{1, 2, 3};
            EXPECT_EQ(vector[0], 1);
            EXPECT_EQ(vector[1], 2);
            EXPECT_EQ(vector[2], 3);
        }

        TEST(Vector, At)
        {
            const auto vector = Vector{1, 2, 3};
            EXPECT_EQ(vector.at(0), 1);
            EXPECT_EQ(vector.at(1), 2);
            EXPECT_EQ(vector.at(2), 3);
        }

        TEST(Vector, AtOutOfRange)
        {
            const auto vector = Vector{1, 2, 3};
            EXPECT_THROW(vector.at(3), std::out_of_range);
        }

        TEST(Vector, Front)
        {
            const auto vector = Vector{1, 2, 3};
            EXPECT_EQ(vector.front(), 1);
        }

        TEST(Vector, Back)
        {
            const auto vector = Vector{1, 2, 3};
            EXPECT_EQ(vector.back(), 3);
        }

        TEST(Vector, EqualEqual)
        {
            const auto lhs = Vector{1, 2, 3};
            const auto rhs = Vector{1, 2, 3};
            ASSERT_EQ(lhs, rhs);
        }

        TEST(Vector, NotEqual)
        {
            const auto lhs = Vector{1, 2, 3};
            const auto rhs = Vector{4, 5, 6};
            ASSERT_NE(lhs, rhs);
        }

        TEST(Vector, Plus)
        {
            const auto lhs = Vector{1, 2, 3};
            const auto rhs = Vector{4, 5, 6};
            const auto result = lhs + rhs;
            const auto expected = Vector{5, 7, 9};
            EXPECT_EQ(result, expected);
        }

        TEST(Vector, PlusEqual)
        {
            auto lhs = Vector{1, 2, 3};
            const auto rhs = Vector{4, 5, 6};
            lhs += rhs;
            const auto expected = Vector{5, 7, 9};
            EXPECT_EQ(lhs, expected);
        }

        TEST(Vector, MinusEqual)
        {
            auto lhs = Vector{1, 2, 3};
            const auto rhs = Vector{4, 5, 6};
            lhs -= rhs;
            const auto expected = Vector{-3, -3, -3};
            EXPECT_EQ(lhs, expected);
        }

        TEST(Vector, Minus)
        {
            const auto lhs = Vector{1, 2, 3};
            const auto rhs = Vector{4, 5, 6};
            const auto result = lhs - rhs;
            const auto expected = Vector{-3, -3, -3};
            EXPECT_EQ(result, expected);
        }

        TEST(Vector, ScalarMultiplication)
        {
            const auto vector = Vector{1, 2, 3};
            const auto result = vector * 2;
            const auto expected = Vector{2, 4, 6};
            EXPECT_EQ(result, expected);
        }

        TEST(Vector, ScalarMultiplicationMixedMode)
        {
            const auto vector = Vector{1, 2, 3};
            const auto result = vector * 2.f;
            using result_type = typename decltype(result)::value_type;
            static_assert(std::is_same_v<result_type, float>);
        }

        TEST(Vector, ScalarDivision)
        {
            const auto vector = Vector{2, 4, 6};
            const auto result = vector / 2;
            const auto expected = Vector{1, 2, 3};
            EXPECT_EQ(result, expected);
        }

        TEST(Vector, ScalarDivisionMixedMode)
        {
            const auto vector = Vector{2, 4, 6};
            const auto result = vector / 2.f;
            using result_type = typename decltype(result)::value_type;
            static_assert(std::is_same_v<result_type, float>);
            const auto expected = Vector{1.f, 2.f, 3.f};
            EXPECT_EQ(result, expected);
        }

        TEST(Vector, DotProduct)
        {
            const auto lhs = Vector{1, 2, 3};
            const auto rhs = Vector{4, 5, 6};
            const auto result = dot(lhs, rhs);
            EXPECT_EQ(result, 32);
        }

        TEST(Vector, CrossProduct)
        {
            const auto a = Vector{1, 2, 3};
            const auto b = Vector{4, 5, 6};
            const auto result = cross(a, b);
            const auto expected = Vector{-3, 6, -3};
            EXPECT_EQ(result, expected);
        }

        TEST(Vector, SqrMagnitude)
        {
            const auto vector = Vector{4, 5, 6};
            EXPECT_EQ(vector.sqr_magnitude(), 77);
        }

        TEST(Vector, SqrMagnitude_Zero)
        {
            const auto vector = Vector{0, 0 ,0};
            EXPECT_EQ(vector.sqr_magnitude(), 0);
        }

        TEST(Vector, SqrMagnitude_Negative)
        {
            const auto vector = Vector{1, -2, 3};
            EXPECT_EQ(vector.sqr_magnitude(), 14);
        }

        TEST(Vector, Magnitude)
        {
            const auto vector = Vector{4, 5, 6};
            EXPECT_FLOAT_EQ(vector.magnitude(), 8.77496438739);
        }
    } // namespace
} // namespace orion
