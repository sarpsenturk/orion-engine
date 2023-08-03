#include "orion-utils/bitflag.h"

#include <gtest/gtest.h>

#include <algorithm>

namespace
{
    enum class MyEnum : std::uint8_t {
        None = 0,
        First = 0x1,
        Second = 0x2,
        Third = 0x4,
        ORION_ENABLE_BITWISE
    };

    TEST(Bitwise, Enabled)
    {
        static_assert(orion::bitwise_enum<MyEnum>);
    }

    TEST(Bitwise, ForSetBit)
    {
        const auto value = MyEnum::First | MyEnum::Third;
        bool zero = false;
        bool first = false;
        bool second = false;
        bool third = false;
        orion::for_set_bit(value, [&zero, &first, &second, &third](auto value) {
            switch (value) {
                case MyEnum::None:
                    zero = true;
                    break;
                case MyEnum::First:
                    first = !first;
                    break;
                case MyEnum::Second:
                    second = !second;
                    break;
                case MyEnum::Third:
                    third = !third;
                    break;
                default:
                    break;
            }
        });
        EXPECT_FALSE(zero);
        EXPECT_TRUE(first);
        EXPECT_FALSE(second);
        EXPECT_TRUE(third);
    }
} // namespace
