#include "orion-math/matrix/matrix.h"

#include <gtest/gtest.h>

namespace
{
    TEST(Matrix, TemplateArgs)
    {
        using Matrix = orion::math::Matrix<int, 1, 4>;
        static_assert(std::is_same_v<typename Matrix::value_type, int>);
        static_assert(Matrix::rows == 1);
        static_assert(Matrix::columns == 4);
    }

    TEST(Matrix, Size)
    {
        using NonEmptyMatrix = orion::math::Matrix<int, 1, 1>;
        using EmptyMatrix = orion::math::Matrix<int, 1, 0>;

        const NonEmptyMatrix non_empty_matrix{};
        EXPECT_EQ(non_empty_matrix.size(), 1);
        EXPECT_FALSE(non_empty_matrix.is_empty());

        const EmptyMatrix empty_matrix{};
        EXPECT_EQ(empty_matrix.size(), 0);
        EXPECT_TRUE(empty_matrix.is_empty());
    }

    TEST(Matrix, Accesors)
    {
        using Matrix = orion::math::Matrix<int, 2, 3>;
        static constexpr int expected = 42;

        Matrix matrix{};
        matrix[1][2] = expected;
        EXPECT_EQ(matrix[1][2], expected);
    }

    TEST(Matrix, Initializer)
    {
        using Matrix = orion::math::Matrix<int, 4, 4>;
        const Matrix matrix{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
        int index = 0;
        for (int row = 0; row < matrix.rows; ++row) {
            for (int col = 0; col < matrix.columns; ++col) {
                EXPECT_EQ(matrix[row][col], index++);
            }
        }
    }

    TEST(Matrix, Equality)
    {
        using Matrix = orion::math::Matrix<int, 2, 2>;

        const Matrix first{1, 2, 3};
        const auto copy = first;
        EXPECT_EQ(first, copy);

        const Matrix second{1, 3, 5};
        EXPECT_NE(first, second);
    }

    TEST(Matrix, Negation)
    {
        using Matrix = orion::math::Matrix<int, 2, 2>;
        const Matrix matrix{1, 2, 3};
        const Matrix expected{-1, -2, -3};
        EXPECT_EQ((-matrix), expected);
    }

    TEST(Matrix, Addition)
    {
        using Matrix = orion::math::Matrix<int, 2, 2>;
        const Matrix lhs{1, 2, 3, 4};
        const Matrix rhs{5, 6, 7, 8};
        const Matrix expected{6, 8, 10, 12};
        EXPECT_EQ((lhs + rhs), expected);
        EXPECT_EQ((rhs + lhs), expected);
    }

    TEST(Matrix, Subtraction)
    {
        using Matrix = orion::math::Matrix<int, 2, 2>;
        const Matrix lhs{1, 2, 3, 4};
        const Matrix rhs{5, 6, 7, 8};
        const Matrix expected{-4, -4, -4, -4};
        EXPECT_EQ((lhs - rhs), expected);
    }

    TEST(Matrix, ScalarMultiplication)
    {
        using Matrix = orion::math::Matrix<int, 2, 2>;
        const Matrix matrix{1, 2, 3, 4};
        const Matrix expected{2, 4, 6, 8};
        EXPECT_EQ((matrix * 2), expected);
        EXPECT_EQ((2 * matrix), expected);
    }

    TEST(Matrix, MatrixMultiplicationSquare)
    {
        using Matrix = orion::math::Matrix<int, 2, 2>;
        const Matrix lhs{1, 2, 3, 4};
        const Matrix rhs{5, 6, 7, 8};
        const Matrix expected{19, 22, 43, 50};
        EXPECT_EQ((lhs * rhs), expected);
    }

    TEST(Matrix, MatrixMultiplicationNonSquare)
    {
        using Matrix1 = orion::math::Matrix<int, 1, 4>;
        using Matrix2 = orion::math::Matrix<float, 4, 4>;

        const Matrix1 lhs{1, 2, 3, 4};
        const Matrix2 rhs{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
        const auto result = lhs * rhs;

        using result_type = decltype(result);
        static_assert(result_type::columns == 4);
        static_assert(result_type::rows == 1);
        static_assert(std::is_same_v<typename result_type::value_type, float>);

        const result_type expected{90.f, 100.f, 110.f, 120.f};
        EXPECT_EQ(result, expected);
    }

    TEST(Matrix, Identity)
    {
        using Matrix = orion::math::Matrix<int, 2, 2>;
        const Matrix expected{1, 0, 0, 1};
        EXPECT_EQ(Matrix::identity(), expected);
    }

    TEST(Matrix, TransposeSquare)
    {
        using Matrix = orion::math::Matrix<int, 2, 2>;
        const Matrix matrix{1, 2, 3, 4};
        const Matrix expected{1, 3, 2, 4};
        EXPECT_EQ(matrix.transpose(), expected);
    }

    TEST(Matrix, NonSquare)
    {
        using Matrix = orion::math::Matrix<int, 2, 4>;
        const Matrix matrix{1, 2, 3, 4, 5, 6, 7, 8};
        const auto transposed = matrix.transpose();
        using result_matrix = decltype(transposed);
        static_assert(transposed.rows == matrix.columns);
        static_assert(transposed.columns == matrix.rows);
        static_assert(std::is_same_v<typename Matrix::value_type, typename result_matrix::value_type>);
        const result_matrix expected{1, 5, 2, 6, 3, 7, 4, 8};
        EXPECT_EQ(transposed, expected);
    }
} // namespace
