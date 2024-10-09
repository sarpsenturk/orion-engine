#include "orion/math/matrix/matrix.h"

#include <gtest/gtest.h>

namespace orion
{
    namespace
    {
        TEST(Matrix, Size)
        {
            using M = Matrix<int, 3, 4>;
            static_assert(M::rows == 3 && M::cols == 4, "Matrix size is incorrect");
        }

        TEST(Matrix, Empty)
        {
            static_assert(Matrix<int, 1, 1>::empty == false);
            static_assert(Matrix<int, 0, 0>::empty == true);
            static_assert(Matrix<int, 1, 0>::empty == true);
            static_assert(Matrix<int, 0, 1>::empty == true);
        }

        TEST(Matrix, ElementAccess)
        {
            const auto matrix = Matrix<int, 2, 2>{1, 2, 3, 4};
            EXPECT_EQ(matrix(0, 0), 1);
            EXPECT_EQ(matrix(0, 1), 2);
            EXPECT_EQ(matrix(1, 0), 3);
            EXPECT_EQ(matrix(1, 1), 4);
        }

        TEST(Matrix, At)
        {
            const auto matrix = Matrix<int, 2, 2>{1, 2, 3, 4};
            EXPECT_EQ(matrix.at(0, 0), 1);
            EXPECT_EQ(matrix.at(0, 1), 2);
            EXPECT_EQ(matrix.at(1, 0), 3);
            EXPECT_EQ(matrix.at(1, 1), 4);
        }

        TEST(Matrix, AtOutOfRange)
        {
            const auto matrix = Matrix<int, 2, 2>{1, 2, 3, 4};
            EXPECT_THROW(matrix.at(2, 0), std::out_of_range);
            EXPECT_THROW(matrix.at(0, 2), std::out_of_range);
        }

        TEST(Matrix, EqualEqual)
        {
            const auto lhs = Matrix<int, 2, 2>{1, 2, 3, 4};
            const auto rhs = Matrix<int, 2, 2>{1, 2, 3, 4};
            EXPECT_EQ(lhs, rhs);
        }

        TEST(Matrix, NotEqual)
        {
            const auto lhs = Matrix<int, 2, 2>{1, 2, 3, 4};
            const auto rhs = Matrix<int, 2, 2>{5, 6, 7, 8};
            EXPECT_NE(lhs, rhs);
        }

        TEST(Matrix, Plus)
        {
            const auto lhs = Matrix<int, 2, 2>{1, 2, 3, 4};
            const auto rhs = Matrix<int, 2, 2>{5, 6, 7, 8};
            const auto expected = Matrix<int, 2, 2>{6, 8, 10, 12};
            EXPECT_EQ(lhs + rhs, expected);
        }

        TEST(Matrix, Minus)
        {
            const auto lhs = Matrix<int, 2, 2>{1, 2, 3, 4};
            const auto rhs = Matrix<int, 2, 2>{5, 6, 7, 8};
            const auto expected = Matrix<int, 2, 2>{-4, -4, -4, -4};
            EXPECT_EQ(lhs - rhs, expected);
        }

        TEST(Matrix, ScalarMultiplication)
        {
            const auto matrix = Matrix<int, 2, 2>{1, 2, 3, 4};
            const auto expected = Matrix<int, 2, 2>{2, 4, 6, 8};
            EXPECT_EQ(matrix * 2, expected);
            EXPECT_EQ(2 * matrix, expected);
        }

        TEST(Matrix, ScalarMultiplicationMixedMode)
        {
            const auto matrix = Matrix<int, 2, 2>{1, 2, 3, 4};
            const auto expected = Matrix<float, 2, 2>{2.f, 4.f, 6.f, 8.f};
            static_assert(std::is_same_v<typename decltype(matrix * 2.f)::value_type, float>);
            EXPECT_EQ(matrix * 2.f, expected);
            EXPECT_EQ(2.f * matrix, expected);
        }

        TEST(Matrix, Transpose)
        {
            const auto matrix = Matrix<int, 2, 2>{1, 2, 3, 4};
            const auto expected = Matrix<int, 2, 2>{1, 3, 2, 4};
            EXPECT_EQ(matrix.transpose(), expected);
        }

        TEST(Matrix, TransposeNonSquare)
        {
            const auto matrix = Matrix<int, 2, 4>{1, 2, 3, 4, 5, 6, 7, 8};
            const auto expected = Matrix<int, 4, 2>{1, 5, 2, 6, 3, 7, 4, 8};
            EXPECT_EQ(matrix.transpose(), expected);
        }

        TEST(Matrix, MatrixMultiplication)
        {
            const auto lhs = Matrix<int, 2, 2>{1, 2, 3, 4};
            const auto rhs = Matrix<int, 2, 2>{5, 6, 7, 8};
            const auto expected = Matrix<int, 2, 2>{19, 22, 43, 50};
            EXPECT_EQ(lhs * rhs, expected);
            EXPECT_NE(lhs * rhs, rhs * lhs);
        }

        TEST(Matrix, MatrixMultiplicationNonSquare)
        {
            const auto lhs = Matrix<int, 2, 4>{1, 2, 3, 4, 5, 6, 7, 8};
            const auto rhs = Matrix<int, 4, 4>{9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24};
            const auto expected = Matrix<int, 2, 4>{170, 180, 190, 200, 410, 436, 462, 488};
            EXPECT_EQ(lhs * rhs, expected);
        }

        TEST(Matrix, Identity)
        {
            const auto identity = Matrix<int, 3, 3>::identity();
            const auto expected = Matrix<int, 3, 3>{1, 0, 0, 0, 1, 0, 0, 0, 1};
            EXPECT_EQ(identity, expected);
        }
    } // namespace
} // namespace orion