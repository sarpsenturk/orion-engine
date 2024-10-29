#define ORION_FORCE_SQRT_IMPL
#include "orion/math/sqrt.hpp"

#include <gtest/gtest.h>

namespace orion
{
    namespace
    {
        TEST(Sqrt, NaN)
        {
            EXPECT_TRUE(std::isnan(math::sqrt(-0.1)));
            EXPECT_TRUE(std::isnan(math::sqrt(-INFINITY)));
            EXPECT_TRUE(std::isnan(math::sqrt(NAN)));
        }

        TEST(Sqrt, Zero)
        {
            EXPECT_FLOAT_EQ(math::sqrt(+0.0), 0.0);
            EXPECT_FLOAT_EQ(math::sqrt(-0.0), -0.0);
        }

        TEST(Sqrt, Positive)
        {
            EXPECT_FLOAT_EQ(math::sqrt(0.1), 0.31622776601);
            EXPECT_FLOAT_EQ(math::sqrt(2), 1.41421356237);
            EXPECT_FLOAT_EQ(math::sqrt(4), 2.0);
        }
    } // namespace
} // namespace orion
