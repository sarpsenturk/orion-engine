#include "orion/math/vector.hpp"

#include <gtest/gtest.h>

namespace orion
{
    namespace
    {
        TEST(Vector, Empty)
        {
            EXPECT_TRUE((Vector<int, 0>{}.empty()));
            EXPECT_FALSE((Vector<int, 1>{}.empty()));
        }

        TEST(Vector, Size)
        {
            EXPECT_EQ((Vector<int, 0>{}.size()), 0);
            EXPECT_EQ((Vector<int, 1>{}.size()), 1);
        }

        TEST(Vector, At)
        {
            // Const element access
            const auto cvec = Vector<int, 3>{1, 2, 3};
            EXPECT_EQ(cvec.at(0), 1);
            EXPECT_EQ(cvec.at(1), 2);
            EXPECT_EQ(cvec.at(2), 3);

            // Non-const element access
            auto vec = Vector<int, 3>{1, 2, 3};
            EXPECT_EQ(vec.at(0) = 4, 4);
            EXPECT_EQ(vec.at(1) = 5, 5);
            EXPECT_EQ(vec.at(2) = 6, 6);

            // Out of range
            const auto empty = Vector<int, 0>{};
            EXPECT_THROW(empty.at(0), std::out_of_range);
        }

        TEST(Vector, Subscript)
        {
            // Const element access
            const auto cvec = Vector<int, 3>{1, 2, 3};
            EXPECT_EQ(cvec[0], 1);
            EXPECT_EQ(cvec[1], 2);
            EXPECT_EQ(cvec[2], 3);

            // Non-const element access
            auto vec = Vector<int, 3>{1, 2, 3};
            EXPECT_EQ(vec[0] = 4, 4);
            EXPECT_EQ(vec[1] = 5, 5);
            EXPECT_EQ(vec[2] = 6, 6);

            // TODO: Test out of range assertions
        }

        TEST(Vector, Data)
        {
            // Const pointer
            const auto cvec = Vector<int, 3>{1, 2, 3};
            const int* cptr = cvec.data();
            EXPECT_NE(cptr, nullptr);
            EXPECT_EQ(*cptr, 1);
            EXPECT_EQ(*(cptr + 1), 2);
            EXPECT_EQ(*(cptr + 2), 3);

            // Non-const pointer
            auto vec = Vector<int, 3>{1, 2, 3};
            int* ptr = vec.data();
            EXPECT_NE(ptr, nullptr);
            EXPECT_EQ(*ptr = 4, 4);
            EXPECT_EQ(*(ptr + 1) = 5, 5);
            EXPECT_EQ(*(ptr + 2) = 6, 6);
        }

        TEST(Vector, Equals)
        {
            const auto a = Vector<int, 3>{1, 2, 3};
            const auto b = Vector<int, 3>{4, 5, 6};
            EXPECT_EQ(a, a);
            EXPECT_NE(a, b);
        }

        TEST(Vector, Add)
        {
            const auto a = Vector<int, 3>{1, 2, 3};
            const auto b = Vector<int, 3>{4, 5, 6};
            const auto expected = Vector<int, 3>{5, 7, 9};
            EXPECT_EQ(a + b, expected);
            EXPECT_EQ(b + a, expected);
        }

        TEST(Vector, Subtract)
        {
            const auto a = Vector<int, 3>{1, 2, 3};
            const auto b = Vector<int, 3>{4, 5, 6};
            const auto expected = Vector<int, 3>{-3, -3, -3};
            EXPECT_EQ(a - b, expected);
            EXPECT_NE(b - a, expected);
        }

        TEST(Vector, Negate)
        {
            const auto vector = Vector<int, 3>{1, 2, 3};
            const auto expected = Vector<int, 3>{-1, -2, -3};
            EXPECT_EQ(-vector, expected);
            EXPECT_EQ(-(-vector), vector);
        }

        TEST(Vector, ScalarMult)
        {
            const auto vector = Vector<int, 3>{1, 2, 3};
            const auto expected = Vector<int, 3>{2, 4, 6};
            const auto promoted = Vector<float, 3>{0.5f, 1.0f, 1.5f};
            EXPECT_EQ(vector * 2, expected);
            EXPECT_EQ(vector * 0.5f, promoted);
        }

        TEST(Vector, ScalarDiv)
        {
            const auto vector = Vector<int, 3>{1, 2, 3};
            const auto expected = Vector<float, 3>{0.5f, 1.0f, 1.5f};
            EXPECT_EQ(vector / 2.0f, expected);
        }

        TEST(Vector, Dot)
        {
            const auto a = Vector<int, 3>{1, 2, 3};
            const auto b = Vector<int, 3>{4, 5, 6};
            EXPECT_EQ(dot(a, b), 32);
            EXPECT_EQ(dot(b, a), 32);
        }

        TEST(Vector, Cross)
        {
            const auto lhs = Vector<int, 3>{1, 2, 3};
            const auto rhs = Vector<int, 3>{4, 5, 6};
            const auto expected = Vector<int, 3>{-3, 6, -3};
            EXPECT_EQ(cross(lhs, rhs), expected);
            EXPECT_NE(cross(lhs, rhs), cross(rhs, lhs));
        }

        TEST(Vector, Magnitude)
        {
            const auto vector = Vector<int, 3>{1, 2, 3};
            EXPECT_EQ(vector.sqr_magnitude(), 14);
            EXPECT_FLOAT_EQ(vector.magnitude(), std::sqrt(14));
        }

        TEST(Vector, Normalized)
        {
            {
                const auto vector = Vector<float, 2>{3.0f, 4.0f};
                const auto expected = Vector<float, 2>{0.6f, 0.8f};
                const auto result = normalize(vector);
                EXPECT_FLOAT_EQ(result[0], expected[0]);
                EXPECT_FLOAT_EQ(result[1], expected[1]);
            }

            {
                const auto vector = Vector<float, 3>{1.0f, 2.0f, 2.0f};
                const auto expected = Vector<float, 3>{1.0f / 3, 2.0f / 3, 2.0f / 3};
                const auto result = normalize(vector);
                EXPECT_FLOAT_EQ(result[0], expected[0]);
                EXPECT_FLOAT_EQ(result[1], expected[1]);
                EXPECT_FLOAT_EQ(result[2], expected[2]);
            }
        }

        TEST(Vector, CTAD)
        {
            const auto vectori = Vector{1, 2, 3};
            EXPECT_EQ(vectori.size, 3);
            EXPECT_TRUE((std::is_same_v<typename decltype(vectori)::value_type, int>));

            const auto vectorf = Vector{1.0f, 2.0f, 3.0f};
            EXPECT_EQ(vectorf.size, 3);
            EXPECT_TRUE((std::is_same_v<typename decltype(vectorf)::value_type, float>));
        }
    } // namespace
} // namespace orion