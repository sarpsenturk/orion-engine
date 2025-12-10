#include "orion/math/matrix.hpp"

#include <gtest/gtest.h>

namespace orion
{
    namespace
    {
        TEST(Matrix, Size)
        {
            EXPECT_EQ((Matrix<int, 2, 4>::rows), 2);
            EXPECT_EQ((Matrix<int, 2, 4>::cols), 4);
            EXPECT_EQ((Matrix<int, 4, 4>::rows), 4);
            EXPECT_EQ((Matrix<int, 4, 2>::cols), 2);
        }

        TEST(Matrix, At)
        {
            // Const element access
            const auto cmatrix = Matrix<int, 2, 2>{1, 2,
                                                   3, 4};
            EXPECT_EQ(cmatrix.at(0, 0), 1);
            EXPECT_EQ(cmatrix.at(0, 1), 2);
            EXPECT_EQ(cmatrix.at(1, 0), 3);
            EXPECT_EQ(cmatrix.at(1, 1), 4);

            // Non-const elment access
            auto matrix = Matrix<int, 2, 2>{};
            EXPECT_EQ(matrix.at(0, 0) = 5, 5);
            EXPECT_EQ(matrix.at(0, 1) = 6, 6);
            EXPECT_EQ(matrix.at(1, 0) = 7, 7);
            EXPECT_EQ(matrix.at(1, 1) = 8, 8);

            // Out of range
            EXPECT_THROW((Matrix<int, 0, 0>{}.at(0, 0)), std::out_of_range);
        }

        TEST(Matrix, Subscript)
        {
            // Const element access
            const auto cmatrix = Matrix<int, 2, 2>{1, 2,
                                                   3, 4};
            EXPECT_EQ(cmatrix(0, 0), 1);
            EXPECT_EQ(cmatrix(0, 1), 2);
            EXPECT_EQ(cmatrix(1, 0), 3);
            EXPECT_EQ(cmatrix(1, 1), 4);

            // Non-const elment access
            auto matrix = Matrix<int, 2, 2>{};
            EXPECT_EQ(matrix(0, 0) = 5, 5);
            EXPECT_EQ(matrix(0, 1) = 6, 6);
            EXPECT_EQ(matrix(1, 0) = 7, 7);
            EXPECT_EQ(matrix(1, 1) = 8, 8);

            // TODO: Test out of range assertions
        }

        TEST(Matrix, Data)
        {
            // Const pointer
            const auto cmatrix = Matrix<int, 2, 2>{1, 2,
                                                   3, 4};
            const int* cptr = cmatrix.data();
            EXPECT_NE(cptr, nullptr);
            EXPECT_EQ(*cptr, 1);
            EXPECT_EQ(*(cptr + 1), 2);
            EXPECT_EQ(*(cptr + 2), 3);
            EXPECT_EQ(*(cptr + 3), 4);

            // Non-const pointer
            auto matrix = Matrix<int, 2, 2>{1, 2,
                                            3, 4};
            int* ptr = matrix.data();
            EXPECT_NE(ptr, nullptr);
            EXPECT_EQ(*ptr = 5, 5);
            EXPECT_EQ(*(ptr + 1) = 6, 6);
            EXPECT_EQ(*(ptr + 2) = 7, 7);
            EXPECT_EQ(*(ptr + 3) = 8, 8);
        }

        TEST(Matrix, Equals)
        {
            const auto a = Matrix<int, 2, 2>{1, 2,
                                             3, 4};
            const auto b = Matrix<int, 2, 2>{5, 6,
                                             7, 8};
            EXPECT_EQ(a, a);
            EXPECT_NE(a, b);
        }

        TEST(Matrix, Add)
        {
            const auto lhs = Matrix<int, 2, 2>{1, 2,
                                               3, 4};
            const auto rhs = Matrix<int, 2, 2>{5, 6,
                                               7, 8};
            const auto expected = Matrix<int, 2, 2>{6, 8,
                                                    10, 12};
            EXPECT_EQ(lhs + rhs, expected);
            EXPECT_EQ(rhs + lhs, expected);
        }

        TEST(Matrix, Subtract)
        {
            const auto lhs = Matrix<int, 2, 2>{1, 2,
                                               3, 4};
            const auto rhs = Matrix<int, 2, 2>{5, 6,
                                               7, 8};
            const auto expected = Matrix<int, 2, 2>{-4, -4,
                                                    -4, -4};
            EXPECT_EQ(lhs - rhs, expected);
            EXPECT_NE(rhs - lhs, expected);
        }

        TEST(Matrix, ScalarMult)
        {
            const auto matrix = Matrix<int, 2, 2>{1, 2, 3, 4};
            const auto expected = Matrix<int, 2, 2>{2, 4, 6, 8};
            const auto promoted = Matrix<float, 2, 2>{0.5f, 1.0f, 1.5f, 2.0f};
            EXPECT_EQ(matrix * 2, expected);
            EXPECT_EQ(matrix * 0.5f, promoted);
        }

        TEST(Matrix, ScalarDiv)
        {
            const auto matrix = Matrix<int, 2, 2>{1, 2,
                                                  3, 4};
            const auto expected = Matrix<float, 2, 2>{0.5f, 1.0f,
                                                      1.5f, 2.0f};
            EXPECT_EQ(matrix / 2.0f, expected);
        }

        TEST(Matrix, MatrixMul)
        {
            // Identity
            {
                const auto matrix = Matrix<int, 3, 3>{1, 2, 3,
                                                      4, 5, 6,
                                                      7, 8, 9};
                const auto identity = Matrix<int, 3, 3>::identity();
                EXPECT_EQ(matrix * identity, matrix);
                EXPECT_EQ(identity * matrix, matrix);
            }

            // Square matrices
            {
                const auto lhs = Matrix<int, 2, 2>{1, 2,
                                                   3, 4};
                const auto rhs = Matrix<int, 2, 2>{5, 6,
                                                   7, 8};
                const auto expected = Matrix<int, 2, 2>{19, 22,
                                                        43, 50};
                EXPECT_EQ(lhs * rhs, expected);
            }

            // Non-square matrices
            {
                const auto lhs = Matrix<int, 2, 3>{1, 2, 3,
                                                   4, 5, 6};
                const auto rhs = Matrix<int, 3, 2>{7, 8,
                                                   9, 10,
                                                   11, 12};
                const auto expected = Matrix<int, 2, 2>{58, 64,
                                                        139, 154};
                EXPECT_EQ(lhs * rhs, expected);
            }

            // Zero matrix
            {
                const auto matrix = Matrix<int, 2, 2>{1, 2,
                                                      3, 4};
                const auto zero = Matrix<int, 2, 2>{};
                EXPECT_EQ(matrix * zero, zero);
                EXPECT_EQ(zero * matrix, zero);
            }
        }

        TEST(Matrix, MatrixMulVector)
        {
            // Column vector
            {
                const auto matrix = Matrix<int, 2, 2>{1, 2,
                                                      3, 4};
                const auto vector = Vector<int, 2>{5, 6};
                const auto expected = Vector<int, 2>{17, 39};
                EXPECT_EQ(matrix * vector, expected);
            }
        }

        TEST(Matrix, GetRow)
        {
            // Square matrix
            {
                const auto matrix = Matrix<int, 2, 2>{1, 2,
                                                      3, 4};
                EXPECT_EQ(matrix.row(0), (Vector{1, 2}));
                EXPECT_EQ(matrix.row(1), (Vector{3, 4}));
            }

            // Non-square matrix
            {
                const auto matrix = Matrix<int, 2, 3>{1, 2, 3,
                                                      4, 5, 6};
                EXPECT_EQ(matrix.row(0), (Vector{1, 2, 3}));
                EXPECT_EQ(matrix.row(1), (Vector{4, 5, 6}));
            }
        }

        TEST(Matrix, GetColumn)
        {
            // Square matrix
            {
                const auto matrix = Matrix<int, 2, 2>{1, 2,
                                                      3, 4};
                EXPECT_EQ(matrix.column(0), (Vector{1, 3}));
                EXPECT_EQ(matrix.column(1), (Vector{2, 4}));
            }

            // Non-square matrix
            {
                const auto matrix = Matrix<int, 2, 3>{1, 2, 3,
                                                      4, 5, 6};
                EXPECT_EQ(matrix.column(0), (Vector{1, 4}));
                EXPECT_EQ(matrix.column(1), (Vector{2, 5}));
                EXPECT_EQ(matrix.column(2), (Vector{3, 6}));
            }
        }

        TEST(Matrix, Transpose)
        {
            // Square matrix
            {
                const auto matrix = Matrix<int, 2, 2>{1, 2,
                                                      3, 4};
                const auto expected = Matrix<int, 2, 2>{1, 3,
                                                        2, 4};
                EXPECT_EQ(matrix.transpose(), expected);
            }

            // Non-square matrix
            {
                const auto matrix = Matrix<int, 2, 3>{1, 2, 3,
                                                      4, 5, 6};
                const auto expected = Matrix<int, 3, 2>{1, 4,
                                                        2, 5,
                                                        3, 6};
                EXPECT_EQ(matrix.transpose(), expected);
            }
            {
                const auto matrix = Matrix<int, 3, 2>{1, 2,
                                                      3, 4,
                                                      5, 6};
                const auto expected = Matrix<int, 2, 3>{1, 3, 5,
                                                        2, 4, 6};
                EXPECT_EQ(matrix.transpose(), expected);
            }

            // 1xN & Nx1
            {
                const auto matrix = Matrix<int, 4, 1>{1,
                                                      2,
                                                      3,
                                                      4};
                const auto expected = Matrix<int, 1, 4>{1, 2, 3, 4};
                EXPECT_EQ(matrix.transpose(), expected);
            }
            {
                const auto matrix = Matrix<int, 1, 4>{1, 2, 3, 4};
                const auto expected = Matrix<int, 4, 1>{1,
                                                        2,
                                                        3,
                                                        4};
                EXPECT_EQ(matrix.transpose(), expected);
            }

            // Identity
            {
                const auto identity = Matrix<int, 3, 3>::identity();
                EXPECT_EQ(identity.transpose(), identity);
            }
        }
    } // namespace
} // namespace orion