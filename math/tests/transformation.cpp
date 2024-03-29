#include "orion-math/matrix/transformation.h"

#include <gtest/gtest.h>

namespace
{
    constexpr auto acceptable_error = 1e-7;

    TEST(Transformation, Transform)
    {
        const auto vector = orion::Vector3_f{1.f, 2.f, 3.f};
        const auto transformation = orion::Matrix4::identity();
        const auto transformed = orion::transform(vector, transformation);
        EXPECT_EQ(transformed, vector);
    }

    TEST(Transformation, Scaling)
    {
        const auto vector = orion::Vector3_f{1.f, 2.f, 3.f};
        const auto scaling = orion::scaling(2.f, 2.f, 2.f);
        const auto transformed = orion::transform(vector, scaling);
        const auto expected = orion::vec3(2.f, 4.f, 6.f);
        EXPECT_EQ(transformed, expected);
    }

    TEST(Transformation, Translation)
    {
        const auto vector = orion::Vector3_f{1.f, 2.f, 3.f};
        const auto translation = orion::translation(3.f, 3.f, 3.f);
        const auto transformed = orion::transform(vector, translation);
        const auto expected = orion::vec3(4.f, 5.f, 6.f);
        EXPECT_EQ(transformed, expected);
    }

    TEST(Transformation, RotationX)
    {
        const auto vector = orion::Vector3_f{0.f, 0.f, 1.f};
        const auto rotation = orion::rotation_x(orion::degrees(90.f));
        const auto transformed = orion::transform(vector, rotation);
        EXPECT_EQ(transformed[0], 0);
        EXPECT_NEAR(transformed[1], -1.f, acceptable_error);
        EXPECT_NEAR(transformed[2], 0.f, acceptable_error);
    }

    TEST(Transformation, RotationY)
    {
        const auto vector = orion::Vector3_f{1.f, 0.f, 0.f};
        const auto rotation = orion::rotation_y(orion::degrees(90.f));
        const auto transformed = orion::transform(vector, rotation);
        EXPECT_NEAR(transformed[0], 0, acceptable_error);
        EXPECT_EQ(transformed[1], 0);
        EXPECT_NEAR(transformed[2], -1, acceptable_error);
    }

    TEST(Transformation, RotationZ)
    {
        const auto vector = orion::Vector3_f{0.f, 1.f, 0.f};
        const auto rotation = orion::rotation_z(orion::degrees(90.f));
        const auto transformed = orion::transform(vector, rotation);
        EXPECT_NEAR(transformed[0], -1, acceptable_error);
        EXPECT_NEAR(transformed[1], 0, acceptable_error);
        EXPECT_EQ(transformed[2], 0);
    }

    TEST(Transformation, LookAtRH)
    {
        const auto position = orion::Vector3_f{0.f, 0.f, 0.f};
        const auto eye = orion::Vector3_f{1.f, 0.f, 1.f};
        const auto target = orion::Vector3_f{eye + orion::Vector3_f{0.f, 0.f, -1.f}};
        const auto up = orion::Vector3_f{0.f, 1.f, 0.f};
        const auto view = orion::lookat_rh(eye, target, up);
        const auto position_view_space = orion::transform(position, view);
        EXPECT_NEAR(position_view_space.x(), -1.f, acceptable_error);
        EXPECT_NEAR(position_view_space.y(), 0.f, acceptable_error);
        EXPECT_NEAR(position_view_space.z(), -1.f, acceptable_error);
    }

    TEST(Transformation, LookAtLH)
    {
        const auto position = orion::vec3(0.f, 0.f, 0.f);
        const auto eye = orion::vec3(1.f, 0.f, -1.f);
        const auto target{eye + orion::vec3(0.f, 0.f, 1.f)};
        const auto up = orion::vec3(0.f, 1.f, 0.f);
        const auto view = orion::lookat_lh(eye, target, up);
        const auto position_view_space = orion::transform(position, view);
        EXPECT_NEAR(position_view_space.x(), -1, acceptable_error);
        EXPECT_NEAR(position_view_space.y(), 0, acceptable_error);
        EXPECT_NEAR(position_view_space.z(), 1, acceptable_error);
    }
} // namespace
