#include "orion-math/abs.h"

#include <gtest/gtest.h>

TEST(Abs, PositiveFloating)
{
    EXPECT_EQ(orion::math::abs(15.f), 15.f);
}

TEST(Abs, NegativeFloating)
{
    EXPECT_EQ(orion::math::abs(-15.f), 15.f);
}

TEST(Abs, PositiveIntegral)
{
    EXPECT_EQ(orion::math::abs(15), 15.0);
}

TEST(Abs, NegativeIntegral)
{
    EXPECT_EQ(orion::math::abs(-15), 15.0);
}

TEST(Abs, PositiveZero)
{
    EXPECT_EQ(orion::math::abs(0.0), 0.0);
}

TEST(Abs, NegativeZero)
{
    EXPECT_EQ(orion::math::abs(-0.0), 0.0);
}
