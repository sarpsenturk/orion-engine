#include "orion/utility/contains.h"

#include <gtest/gtest.h>

namespace
{
    TEST(Types, Contains)
    {
        constexpr auto expect_true =
            orion::types::contains_v<int, float, const int, double, int, char>;
        EXPECT_TRUE(expect_true);

        constexpr auto expect_false = orion::types::contains_v<char, int, float, double>;
        EXPECT_FALSE(expect_false);
    }
} // namespace
