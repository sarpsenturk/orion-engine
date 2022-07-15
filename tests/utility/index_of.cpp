#include "orion/utility/index_of.h"

#include <gtest/gtest.h>

namespace
{
    TEST(IndexOf, BasicCheck)
    {
        constexpr auto index =
            orion::detail::index_of_v<int, float, const int, double, int, char>;
        EXPECT_EQ(3, index);
    }
} // namespace
