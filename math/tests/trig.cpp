#include "orion-math/trig.h"

#include <gtest/gtest.h>

namespace
{
    constexpr auto acceptable_error = 1e-5;
    constexpr auto three_halfs_pi = 3 * orion::pi_radians / 2;

    TEST(Trig, Sin)
    {
        EXPECT_EQ(orion::sin(orion::radians(0.0)), 0.0);
        EXPECT_NEAR(orion::sin(orion::pi_radians), 0.0, acceptable_error);
        EXPECT_NEAR(orion::sin(orion::pi_radians), 0.0, acceptable_error);
        EXPECT_NEAR(orion::sin(orion::radians(3 * orion::pi / 2)), -1.0, acceptable_error);
        EXPECT_NEAR(orion::sin(orion::radians(3 * -orion::pi / 2)), 1.0, acceptable_error);
        EXPECT_NEAR(orion::sin(orion::radians(4.0)), -0.7568024953, acceptable_error);
        EXPECT_NEAR(orion::sin(orion::radians(-4.0)), 0.7568024953, acceptable_error);
    }

    TEST(Trig, Cos)
    {
        EXPECT_EQ(orion::cos(orion::radians(0.0)), 1.0);
        EXPECT_NEAR(orion::cos(orion::pi_radians), -1.0, acceptable_error);
        EXPECT_NEAR(orion::cos(-orion::pi_radians), -1.0, acceptable_error);
        EXPECT_NEAR(orion::cos(three_halfs_pi), 0.0, acceptable_error);
        EXPECT_NEAR(orion::cos(-three_halfs_pi), 0.0, acceptable_error);
        EXPECT_NEAR(orion::cos(orion::radians(4.0)), -0.65364362086, acceptable_error);
        EXPECT_NEAR(orion::cos(orion::radians(-4.0)), -0.65364362086, acceptable_error);
    }
} // namespace
