#include "orion-utils/array.h"

#include <gtest/gtest.h>

namespace orion
{
    namespace
    {
        TEST(Array, MakeArrayValue)
        {
            constexpr auto array = make_array<5>(42);
            static_assert(array.size() == 5);
            for (auto value : array) {
                EXPECT_EQ(value, 42);
            }
        }

        TEST(Array, MakeArrayGenerator)
        {
            constexpr auto array = make_array<5>([]() { return 42; });
            static_assert(array.size() == 5);
            for (auto value : array) {
                EXPECT_EQ(value, 42);
            }
        }

        TEST(Array, MakeArrayIndexed)
        {
            constexpr auto array = make_array<5>([](int index) { return index; });
            static_assert(array.size() == 5);
            static_assert(std::is_same_v<typename decltype(array)::value_type, int>);
            for (int i = 0; i < array.size(); ++i) {
                EXPECT_EQ(array[i], i);
            }
        }
    } // namespace
} // namespace orion
