#include "orion-math/matrix/transformation.h"

#include <gtest/gtest.h>

namespace
{
    constexpr auto acceptable_error = 1e-7;

    TEST(Transformation, Transform)
    {
        const orion::math::Vector3 vector{1, 2, 3};
        const auto transformation = orion::math::Matrix4::identity();
        const auto transformed = orion::math::transform(vector, transformation);
        EXPECT_EQ(transformed, vector);
    }

    TEST(Transformation, Scaling)
    {
        const orion::math::Vector3 vector{1, 2, 3};
        const auto scaling = orion::math::scaling(2.f, 2.f, 2.f);
        const auto transformed = orion::math::transform(vector, scaling);
        const orion::math::Vector3 expected{2.f, 4.f, 6.f};
        EXPECT_EQ(transformed, expected);
    }

    TEST(Transformation, Translation)
    {
        const orion::math::Vector3 vector{1, 2, 3};
        const auto translation = orion::math::translation(3.f, 3.f, 3.f);
        const auto transformed = orion::math::transform(vector, translation);
        const orion::math::Vector3 expected{4.f, 5.f, 6.f};
        EXPECT_EQ(transformed, expected);
    }

    TEST(Transformation, RotationX)
    {
        using namespace orion::math::angle_literals;
        const orion::math::Vector3 vector{0, 0, 1};
        const auto rotation = orion::math::rotation_x(90_deg);
        const auto transformed = orion::math::transform(vector, rotation);
        EXPECT_EQ(transformed[0], 0);
        EXPECT_NEAR(transformed[1], -1, acceptable_error);
        EXPECT_NEAR(transformed[2], 0, acceptable_error);
    }

    TEST(Transformation, RotationY)
    {
        using namespace orion::math::angle_literals;
        const orion::math::Vector3 vector{1, 0, 0};
        const auto rotation = orion::math::rotation_y(90_deg);
        const auto transformed = orion::math::transform(vector, rotation);
        EXPECT_NEAR(transformed[0], 0, acceptable_error);
        EXPECT_EQ(transformed[1], 0);
        EXPECT_NEAR(transformed[2], -1, acceptable_error);
    }

    TEST(Transformation, RotationZ)
    {
        using namespace orion::math::angle_literals;
        const orion::math::Vector3 vector{0, 1, 0};
        const auto rotation = orion::math::rotation_z(90_deg);
        const auto transformed = orion::math::transform(vector, rotation);
        EXPECT_NEAR(transformed[0], -1, acceptable_error);
        EXPECT_NEAR(transformed[1], 0, acceptable_error);
        EXPECT_EQ(transformed[2], 0);
    }

    TEST(Transformation, LookAtRH)
    {
        const orion::math::Vector3 position{0, 0, 0};
        const orion::math::Vector3 eye{1, 0, 1};
        const orion::math::Vector3 target{eye + orion::math::Vector3{0, 0, -1}};
        const orion::math::Vector3 up{0, 1, 0};
        const auto view = orion::math::lookat_rh(eye, target, up);
        const auto position_view_space = orion::math::transform(position, view);
        EXPECT_NEAR(position_view_space.x(), -1, acceptable_error);
        EXPECT_NEAR(position_view_space.y(), 0, acceptable_error);
        EXPECT_NEAR(position_view_space.z(), -1, acceptable_error);
    }

    TEST(Transformation, LookAtLH)
    {
        const orion::math::Vector3 position{0, 0, 0};
        const orion::math::Vector3 eye{1, 0, -1};
        const orion::math::Vector3 target{eye + orion::math::Vector3{0, 0, 1}};
        const orion::math::Vector3 up{0, 1, 0};
        const auto view = orion::math::lookat_lh(eye, target, up);
        const auto position_view_space = orion::math::transform(position, view);
        EXPECT_NEAR(position_view_space.x(), -1, acceptable_error);
        EXPECT_NEAR(position_view_space.y(), 0, acceptable_error);
        EXPECT_NEAR(position_view_space.z(), 1, acceptable_error);
    }

    TEST(Transformation, OrthographicRH)
    {
        const orion::math::Vector3 position{2.5f, -2.5f, -5.f};
        const auto projection = orion::math::orthographic_rh(-5.f, 5.f, -5.f, 5.f, 0.f, 10.f);
        const auto position_projected = orion::math::transform(position, projection);
        EXPECT_NEAR(position_projected.x(), .5f, acceptable_error);
        EXPECT_NEAR(position_projected.y(), -.5f, acceptable_error);
        EXPECT_NEAR(position_projected.z(), .5f, acceptable_error);
    }

    TEST(Transformation, OrthographicLH)
    {
        const orion::math::Vector3 position{2.5f, -2.5f, -5.f};
        const auto projection = orion::math::orthographic_lh(-5.f, 5.f, -5.f, 5.f, 0.f, 10.f);
        const auto position_projected = orion::math::transform(position, projection);
        EXPECT_NEAR(position_projected.x(), .5f, acceptable_error);
        EXPECT_NEAR(position_projected.y(), -.5f, acceptable_error);
        EXPECT_NEAR(position_projected.z(), -.5f, acceptable_error);
    }
} // namespace
