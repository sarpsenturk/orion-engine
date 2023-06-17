#include "orion-utils/bitflag.h"

#include <gtest/gtest.h>

namespace
{
    enum class MyEnum : std::uint8_t {
        First,
        Second,
        Third,
    };
    using MyEnumFlags = orion::Bitflag<MyEnum>;

    TEST(Bitflag, DefaultCtor)
    {
        const auto flags = MyEnumFlags{};
        EXPECT_TRUE(flags.has_none());
    }

    TEST(Bitflag, SetBitCtor)
    {
        static constexpr auto expected = MyEnum::Second;
        const auto flags = MyEnumFlags{expected};
        EXPECT_EQ(flags.value(), 1u << orion::to_underlying(expected));
    }

    TEST(Bitflag, ValueCtor)
    {
        static constexpr auto expected = std::uint8_t{8};
        const auto flags = MyEnumFlags{expected};
        EXPECT_EQ(flags.value(), expected);
    }

    TEST(Bitflag, FactoryAll)
    {
        EXPECT_TRUE(MyEnumFlags::all().has_all());
    }

    TEST(Bitflag, FactoryNone)
    {
        EXPECT_TRUE(MyEnumFlags::none().has_none());
    }

    TEST(Bitflag, FactoryDisjunction)
    {
        static constexpr auto first = MyEnum::First;
        static constexpr auto second = MyEnum::Second;

        // empty or
        {
            const auto flags = MyEnumFlags::disjunction({});
            EXPECT_TRUE(flags.has_none());
        }

        // first or first
        {
            const auto flags = MyEnumFlags::disjunction({first, first});
            EXPECT_EQ(flags, first);
        }

        // first or second
        {
            const auto flags = MyEnumFlags::disjunction({first, second});
            EXPECT_TRUE(flags.has(first));
            EXPECT_TRUE(flags.has(second));
        }
    }

    TEST(Bitflag, FactoryConjunction)
    {
        static constexpr auto first = MyEnum::First;
        static constexpr auto second = MyEnum::Second;

        // empty and
        {
            const auto flags = MyEnumFlags::conjunction({});
            EXPECT_TRUE(flags.has_all());
        }

        // first and first
        {
            const auto flags = MyEnumFlags::conjunction({first, first});
            EXPECT_EQ(flags, first);
        }

        // first and second
        {
            const auto flags = MyEnumFlags::conjunction({first, second});
            EXPECT_FALSE(flags.has(first));
            EXPECT_FALSE(flags.has(second));
        }
    }

    TEST(Bitflag, HasBit)
    {
        static constexpr auto expected = MyEnum::First;
        static constexpr auto unexpected = MyEnum::Second;
        static_assert(expected != unexpected);
        const auto flags = MyEnumFlags{expected};
        EXPECT_TRUE(flags.has(expected));
        EXPECT_FALSE(flags.has(unexpected));
    }

    TEST(Bitflag, Complement)
    {
        EXPECT_EQ(~MyEnumFlags::none(), MyEnumFlags::all());
        EXPECT_EQ(~MyEnumFlags::all(), MyEnumFlags::none());
    }

    TEST(Bitflag, BitwiseOR)
    {
        const auto first = MyEnum::First;
        const auto second = MyEnum::Second;

        // Or with self (first | first)
        {
            const auto first_flag = MyEnumFlags{first};
            const auto bitwise_or = first_flag | first_flag;
            EXPECT_EQ(first_flag, bitwise_or);
        }

        // Or with other (first | second)
        {
            const auto bitwise_or = MyEnumFlags{first} | MyEnumFlags{second};
            EXPECT_TRUE(bitwise_or.has(first));
            EXPECT_TRUE(bitwise_or.has(second));
        }
    }

    TEST(Bitflag, BitwiseAND)
    {
        const auto first = MyEnum::First;
        const auto second = MyEnum::Second;

        // And with self (first & first)
        {
            const auto first_flag = MyEnumFlags{first};
            const auto bitwise_and = first_flag & first_flag;
            EXPECT_EQ(first_flag, bitwise_and);
        }

        // And with other (first & second)
        {
            const auto bitwise_and = MyEnumFlags{first} & MyEnumFlags{second};
            EXPECT_TRUE(bitwise_and.has_none());
        }
    }

    TEST(Bitflag, BitwiseXOR)
    {
        const auto first = MyEnum::First;
        const auto second = MyEnum::Second;

        // Xor with self (first ^ first)
        {
            const auto first_flag = MyEnumFlags{first};
            const auto bitwise_xor = first_flag ^ first_flag;
            EXPECT_TRUE(bitwise_xor.has_none());
        }

        // Xor with other (first ^ second)
        {
            const auto bitwise_xor = MyEnumFlags{first} ^ MyEnumFlags { second };
            EXPECT_TRUE(bitwise_xor.has(first));
            EXPECT_TRUE(bitwise_xor.has(second));
        }
    }

    TEST(Bitflag, BitwiseLeftShift)
    {
        EXPECT_EQ((MyEnumFlags{MyEnum::First} << 1), MyEnumFlags{MyEnum::Second});
        EXPECT_EQ((MyEnumFlags{MyEnum::First} << 2), MyEnumFlags{MyEnum::Third});
    }

    TEST(Bitflag, BitwiseRighttShift)
    {
        EXPECT_EQ((MyEnumFlags{MyEnum::Third} >> 1), MyEnumFlags{MyEnum::Second});
        EXPECT_EQ((MyEnumFlags{MyEnum::Third} >> 2), MyEnumFlags{MyEnum::First});
    }
} // namespace
