#include "orion-math/trig.h"

#include <gtest/gtest.h>

using namespace orion::math::angle_literals;

namespace
{
    constexpr auto acceptable_error = 1e-5;
    constexpr auto three_halfs_pi = 3 * orion::math::pi_rads / 2;

    TEST(Sin, Zero)
    {
        EXPECT_EQ(orion::math::sin(0_rad), 0.0);
    }

    TEST(Sin, PlusMinusPi)
    {
        EXPECT_NEAR(orion::math::sin(orion::math::pi_rads), 0.0, acceptable_error);
        EXPECT_NEAR(orion::math::sin(orion::math::pi_rads), 0.0, acceptable_error);
    }

    TEST(Sin, BetweenPlusMinusPiAnd2Pi)
    {
        EXPECT_NEAR(orion::math::sin(orion::math::Radians{3 * orion::math::pi / 2}), -1.0, acceptable_error);
        EXPECT_NEAR(orion::math::sin(orion::math::Radians{3 * -orion::math::pi / 2}), 1.0, acceptable_error);
    }

    TEST(Sin, AbovePlusMinus2Pi)
    {
        EXPECT_NEAR(orion::math::sin(4_rad), -0.7568024953, acceptable_error);
        EXPECT_NEAR(orion::math::sin(-4_rad), 0.7568024953, acceptable_error);
    }

    TEST(Cos, Zero)
    {
        EXPECT_EQ(orion::math::cos(0_rad), 1.0);
    }

    TEST(Cos, PlusMinusPi)
    {
        EXPECT_NEAR(orion::math::cos(orion::math::pi_rads), -1.0, acceptable_error);
        EXPECT_NEAR(orion::math::cos(-orion::math::pi_rads), -1.0, acceptable_error);
    }

    TEST(Cos, BetweenPlusMinusPiAnd2Pi)
    {
        EXPECT_NEAR(orion::math::cos(three_halfs_pi), 0.0, acceptable_error);
        EXPECT_NEAR(orion::math::cos(-three_halfs_pi), 0.0, acceptable_error);
    }

    TEST(Cos, AbovePlusMinus2Pi)
    {
        EXPECT_NEAR(orion::math::cos(4_rad), -0.65364362086, acceptable_error);
        EXPECT_NEAR(orion::math::cos(-4_rad), -0.65364362086, acceptable_error);
    }
} // namespace
