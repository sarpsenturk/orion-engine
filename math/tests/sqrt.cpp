#include "orion-math/sqrt.h"

#include <gtest/gtest.h>

TEST(Sqrt, IntegerResults)
{
    EXPECT_EQ(orion::math::sqrt(4), 2.0);
    EXPECT_EQ(orion::math::sqrt(16), 4.0);
    EXPECT_EQ(orion::math::sqrt(64), 8.0);
}

TEST(Sqrt, FloatingResults)
{
    EXPECT_DOUBLE_EQ(orion::math::sqrt(2), 1.4142135623730950488);
}

TEST(Sqrt, Zero)
{
    EXPECT_EQ(orion::math::sqrt(0), 0);
}
