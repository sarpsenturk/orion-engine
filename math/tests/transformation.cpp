#include "orion-math/matrix/transformation.h"

#include <gtest/gtest.h>

namespace
{
    constexpr auto acceptable_error = 1e-7;

    TEST(Transformation, Transform)
    {
        const orion::Vector3 vector{1, 2, 3};
        const auto transformation = orion::Matrix4::identity();
        const auto transformed = orion::transform(vector, transformation);
        EXPECT_EQ(transformed, vector);
    }

    TEST(Transformation, Scaling)
    {
        const orion::Vector3 vector{1, 2, 3};
        const auto scaling = orion::scaling(2.f, 2.f, 2.f);
        const auto transformed = orion::transform(vector, scaling);
        const orion::Vector3 expected{2.f, 4.f, 6.f};
        EXPECT_EQ(transformed, expected);
    }

    TEST(Transformation, Translation)
    {
        const orion::Vector3 vector{1, 2, 3};
        const auto translation = orion::translation(3.f, 3.f, 3.f);
        const auto transformed = orion::transform(vector, translation);
        const orion::Vector3 expected{4.f, 5.f, 6.f};
        EXPECT_EQ(transformed, expected);
    }

    TEST(Transformation, RotationX)
    {
        using namespace orion::angle_literals;
        const orion::Vector3 vector{0, 0, 1};
        const auto rotation = orion::rotation_x(90_deg);
        const auto transformed = orion::transform(vector, rotation);
        EXPECT_EQ(transformed[0], 0);
        EXPECT_NEAR(transformed[1], -1, acceptable_error);
        EXPECT_NEAR(transformed[2], 0, acceptable_error);
    }

    TEST(Transformation, RotationY)
    {
        using namespace orion::angle_literals;
        const orion::Vector3 vector{1, 0, 0};
        const auto rotation = orion::rotation_y(90_deg);
        const auto transformed = orion::transform(vector, rotation);
        EXPECT_NEAR(transformed[0], 0, acceptable_error);
        EXPECT_EQ(transformed[1], 0);
        EXPECT_NEAR(transformed[2], -1, acceptable_error);
    }

    TEST(Transformation, RotationZ)
    {
        using namespace orion::angle_literals;
        const orion::Vector3 vector{0, 1, 0};
        const auto rotation = orion::rotation_z(90_deg);
        const auto transformed = orion::transform(vector, rotation);
        EXPECT_NEAR(transformed[0], -1, acceptable_error);
        EXPECT_NEAR(transformed[1], 0, acceptable_error);
        EXPECT_EQ(transformed[2], 0);
    }

    TEST(Transformation, LookAtRH)
    {
        const orion::Vector3 position{0, 0, 0};
        const orion::Vector3 eye{1, 0, 1};
        const orion::Vector3 target{eye + orion::Vector3{0, 0, -1}};
        const orion::Vector3 up{0, 1, 0};
        const auto view = orion::lookat_rh(eye, target, up);
        const auto position_view_space = orion::transform(position, view);
        EXPECT_NEAR(position_view_space.x(), -1, acceptable_error);
        EXPECT_NEAR(position_view_space.y(), 0, acceptable_error);
        EXPECT_NEAR(position_view_space.z(), -1, acceptable_error);
    }

    TEST(Transformation, LookAtLH)
    {
        const orion::Vector3 position{0, 0, 0};
        const orion::Vector3 eye{1, 0, -1};
        const orion::Vector3 target{eye + orion::Vector3{0, 0, 1}};
        const orion::Vector3 up{0, 1, 0};
        const auto view = orion::lookat_lh(eye, target, up);
        const auto position_view_space = orion::transform(position, view);
        EXPECT_NEAR(position_view_space.x(), -1, acceptable_error);
        EXPECT_NEAR(position_view_space.y(), 0, acceptable_error);
        EXPECT_NEAR(position_view_space.z(), 1, acceptable_error);
    }

    TEST(Transformation, OrthographicRH)
    {
        const orion::Vector3 position{2.5f, 2.5f, -2.5f};
        const auto projection = orion::orthographic_rh(0.f, 5.f, 0.f, 5.f, 0.f, 10.f);
        const auto position_projected = orion::transform(position, projection);
        EXPECT_NEAR(position_projected.x(), 0.f, acceptable_error);
        EXPECT_NEAR(position_projected.y(), 0.f, acceptable_error);
        EXPECT_NEAR(position_projected.z(), -.5f, acceptable_error);
    }

    TEST(Transformation, OrthographicLH)
    {
        const orion::Vector3 position{2.5f, 2.5f, 2.5f};
        const auto projection = orion::orthographic_lh(0.f, 5.f, 0.f, 5.f, 0.f, 10.f);
        const auto position_projected = orion::transform(position, projection);
        EXPECT_NEAR(position_projected.x(), 0.f, acceptable_error);
        EXPECT_NEAR(position_projected.y(), 0.f, acceptable_error);
        EXPECT_NEAR(position_projected.z(), -.5f, acceptable_error);
    }
} // namespace
