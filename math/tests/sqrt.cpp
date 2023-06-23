#include "orion-math/sqrt.h"

#include <gtest/gtest.h>

namespace
{
    TEST(Sqrt, IntegerResults)
    {
        EXPECT_EQ(orion::sqrt(4), 2.0);
        EXPECT_EQ(orion::sqrt(16), 4.0);
        EXPECT_EQ(orion::sqrt(64), 8.0);
    }

    TEST(Sqrt, FloatingResults)
    {
        EXPECT_DOUBLE_EQ(orion::sqrt(2), 1.4142135623730950488);
    }

    TEST(Sqrt, Zero)
    {
        EXPECT_EQ(orion::sqrt(0), 0);
    }
} // namespace
