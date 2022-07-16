#include "orion/utility/index_of.h"

#include <gtest/gtest.h>

namespace
{
    TEST(Types, IndexOf)
    {
        constexpr auto index =
            orion::types::index_of_v<int, float, const int, double, int, char>;
        EXPECT_EQ(3, index);
    }
} // namespace
