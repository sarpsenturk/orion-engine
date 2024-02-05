#include "orion-utils/bitflag.h"

#include <gtest/gtest.h>

#include <algorithm>

namespace
{
    enum class MyEnum : std::uint8_t {
        None = 0,
        First = 0x1,
        Second = 0x2,
        Third = 0x4
    };

    template<>
    struct orion::enum_bitwise_enabled<MyEnum> : std::true_type {
    };

    TEST(Bitwise, Enabled)
    {
        static_assert(orion::bitwise_enum<MyEnum>);
    }
} // namespace
