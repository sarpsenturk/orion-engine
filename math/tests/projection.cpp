#include "orion-math/matrix/projection.h"

#include "orion-math/matrix/transformation.h"
#include "orion-math/vector/vector3.h"

#include <gtest/gtest.h>

namespace
{
    constexpr auto acceptable_error = 1e-7;

    TEST(Projection, OrthographicRH)
    {
        const auto position = orion::vec3(2.5f, 2.5f, -2.5f);
        const auto projection = orion::orthographic_rh(0.f, 5.f, 0.f, 5.f, 0.f, 10.f);
        const auto position_projected = orion::transform(position, projection);
        EXPECT_NEAR(position_projected.x(), 0.f, acceptable_error);
        EXPECT_NEAR(position_projected.y(), 0.f, acceptable_error);
        EXPECT_NEAR(position_projected.z(), -.5f, acceptable_error);
    }

    TEST(Projection, OrthographicLH)
    {
        const auto position = orion::vec3(2.5f, 2.5f, 2.5f);
        const auto projection = orion::orthographic_lh(0.f, 5.f, 0.f, 5.f, 0.f, 10.f);
        const auto position_projected = orion::transform(position, projection);
        EXPECT_NEAR(position_projected.x(), 0.f, acceptable_error);
        EXPECT_NEAR(position_projected.y(), 0.f, acceptable_error);
        EXPECT_NEAR(position_projected.z(), -.5f, acceptable_error);
    }
} // namespace
