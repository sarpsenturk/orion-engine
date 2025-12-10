#include "orion/math/transform.hpp"

#include "orion/math/angles.hpp"

#include <gtest/gtest.h>

namespace orion
{
    namespace
    {
        TEST(Transform, TranslateIdentity)
        {
            const auto v = Vector{1, 2, 3, 1};
            const auto t = Vector{0, 0, 0};
            const auto translated = translate(Matrix4i::identity(), t) * v;
            const auto expected = Vector{1, 2, 3, 1};
            EXPECT_EQ(translated, expected);
        }

        TEST(Transform, TranslateX)
        {
            const auto v = Vector{1, 2, 3, 1};
            const auto t = Vector{5, 0, 0};
            const auto translated = translate(Matrix4i::identity(), t) * v;
            const auto expected = Vector{6, 2, 3, 1};
            EXPECT_EQ(translated, expected);
        }

        TEST(Transform, TranslateY)
        {
            const auto v = Vector{1, 2, 3, 1};
            const auto t = Vector{0, 5, 0};
            const auto translated = translate(Matrix4i::identity(), t) * v;
            const auto expected = Vector{1, 7, 3, 1};
            EXPECT_EQ(translated, expected);
        }

        TEST(Transform, TranslateZ)
        {
            const auto v = Vector{1, 2, 3, 1};
            const auto t = Vector{0, 0, 5};
            const auto translated = translate(Matrix4i::identity(), t) * v;
            const auto expected = Vector{1, 2, 8, 1};
            EXPECT_EQ(translated, expected);
        }

        TEST(Transform, Translate)
        {
            const auto v = Vector{1, 2, 3, 1};
            const auto t = Vector{-4, 5, 6};
            const auto translated = translate(Matrix4i::identity(), t) * v;
            const auto expected = Vector{-3, 7, 9, 1};
            EXPECT_EQ(translated, expected);
        }

        TEST(Transform, RotateX)
        {
            {
                const auto v = Vector{0, 1, 0, 1};
                const auto r = radians(90.f);
                const auto rotated = rotate_x(Matrix4i::identity(), r) * v;
                const auto expected = Vector{0, 0, 1, 1};
                EXPECT_EQ(rotated, expected);
            }

            {
                const auto v = Vector{0, 0, 1, 1};
                const auto r = radians(90.f);
                const auto rotated = rotate_x(Matrix4i::identity(), r) * v;
                const auto expected = Vector{0, -1, 0, 1};
                EXPECT_EQ(rotated, expected);
            }

            {
                const auto v = Vector{1, 1, 1, 1};
                const auto r = radians(180.f);
                const auto rotated = rotate_x(Matrix4i::identity(), r) * v;
                const auto expected = Vector{1, -1, -1, 1};
                EXPECT_EQ(rotated, expected);
            }
        }

        TEST(Transform, RotateY)
        {
            {
                const auto v = Vector{1, 0, 0, 1};
                const auto r = radians(90.f);
                const auto rotated = rotate_y(Matrix4i::identity(), r) * v;
                const auto expected = Vector{0, 0, -1, 1};
                EXPECT_EQ(rotated, expected);
            }

            {
                const auto v = Vector{0, 0, 1, 1};
                const auto r = radians(90.f);
                const auto rotated = rotate_y(Matrix4i::identity(), r) * v;
                const auto expected = Vector{1, 0, 0, 1};
                EXPECT_EQ(rotated, expected);
            }

            {
                const auto v = Vector{1, 1, 1, 1};
                const auto r = radians(180.f);
                const auto rotated = rotate_y(Matrix4i::identity(), r) * v;
                const auto expected = Vector{-1, 1, -1, 1};
                EXPECT_EQ(rotated, expected);
            }
        }

        TEST(Transform, RotateZ)
        {
            {
                const auto v = Vector{1, 0, 0, 1};
                const auto r = radians(90.f);
                const auto rotated = rotate_z(Matrix4i::identity(), r) * v;
                const auto expected = Vector{0, 1, 0, 1};
                EXPECT_EQ(rotated, expected);
            }

            {
                const auto v = Vector{0, 1, 0, 1};
                const auto r = radians(90.f);
                const auto rotated = rotate_z(Matrix4i::identity(), r) * v;
                const auto expected = Vector{-1, 0, 0, 1};
                EXPECT_EQ(rotated, expected);
            }

            {
                const auto v = Vector{1, 1, 1, 1};
                const auto r = radians(180.f);
                const auto rotated = rotate_z(Matrix4i::identity(), r) * v;
                const auto expected = Vector{-1, -1, 1, 1};
                EXPECT_EQ(rotated, expected);
            }
        }

        TEST(Transform, ScaleUniform)
        {
            const auto v = Vector{1, 1, 1, 1};
            const auto s = Vector{2, 2, 2};
            const auto scaled = scale(Matrix4i::identity(), s) * v;
            const auto expected = Vector{2, 2, 2, 1};
            EXPECT_EQ(scaled, expected);
        }

        TEST(Transform, ScaleNonUniform)
        {
            const auto v = Vector{1, 1, 1, 1};
            const auto s = Vector{2, 1, 3};
            const auto scaled = scale(Matrix4i::identity(), s) * v;
            const auto expected = Vector{2, 1, 3, 1};
            EXPECT_EQ(scaled, expected);
        }

        TEST(Transform, ScaleZero)
        {
            const auto v = Vector{1, 1, 1, 1};
            const auto s = Vector{0, 0, 0};
            const auto scaled = scale(Matrix4i::identity(), s) * v;
            const auto expected = Vector{0, 0, 0, 1};
            EXPECT_EQ(scaled, expected);
        }

        TEST(Transform, ScaleNegative)
        {
            const auto v = Vector{1, 1, 1, 1};
            const auto s = Vector{-1, 1, 1};
            const auto scaled = scale(Matrix4i::identity(), s) * v;
            const auto expected = Vector{-1, 1, 1, 1};
            EXPECT_EQ(scaled, expected);
        }
    } // namespace
} // namespace orion