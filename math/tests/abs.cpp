#include "orion-math/abs.h"

#include <gtest/gtest.h>

namespace
{
    TEST(Abs, Floating)
    {
        EXPECT_EQ(orion::abs(15.f), 15.f);
        EXPECT_EQ(orion::abs(-15.f), 15.f);
    }

    TEST(Abs, Integral)
    {
        EXPECT_EQ(orion::abs(15), 15.0);
        EXPECT_EQ(orion::abs(-15), 15.0);
    }

    TEST(Abs, Zero)
    {
        EXPECT_EQ(orion::abs(0.0), 0.0);
        EXPECT_EQ(orion::abs(-0.0), 0.0);
    }
} // namespace
