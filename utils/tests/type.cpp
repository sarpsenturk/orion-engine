#include "orion-utils/type.h"

#include <gtest/gtest.h>

namespace
{
    TEST(Type, AnyOf)
    {
        static_assert(orion::any_of<int, char, bool, int, float>);
        static_assert(!orion::any_of<int, char, bool, float>);
    }

    TEST(Type, AllOf)
    {
        static_assert(orion::all_of<int, int, int, int>);
        static_assert(!orion::all_of<int, int, int, bool>);
    }

    TEST(Type, NotEmpty)
    {
        static_assert(orion::not_empty<int>);
        static_assert(!orion::not_empty<>);
    }

    TEST(Type, ToUnderlying)
    {
        using underlying = std::uint8_t;
        enum class Enum : underlying {};
        static_assert(std::is_same_v<underlying, decltype(orion::to_underlying(std::declval<Enum>()))>);
    }
} // namespace
