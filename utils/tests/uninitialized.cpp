#include "orion-utils/uninitialized.h"

#include <gtest/gtest.h>

namespace
{
    struct DefaultConstructable {
        std::string value{"default"};
    };

    struct MoveCopyTest {
        bool default_ctor = true;
        bool copy_ctor = false;
        bool move_ctor = false;

        MoveCopyTest() = default;
        MoveCopyTest(const MoveCopyTest&)
            : default_ctor(false)
            , copy_ctor(true)
            , move_ctor(false)
        {
        }
        MoveCopyTest(MoveCopyTest&&) noexcept
            : default_ctor(false)
            , copy_ctor(false)
            , move_ctor(true)
        {
        }
    };

    TEST(Uninitialized, DefaultContruct)
    {
        constexpr int count{3};
        alignas(alignof(DefaultConstructable)) unsigned char memory[count * sizeof(DefaultConstructable)];

        auto first{reinterpret_cast<DefaultConstructable*>(memory)};
        auto last{first + count};

        orion::uninitialized_default_construct(first, last);

        for (auto it = first; it != last; ++it) {
            EXPECT_EQ(it->value, "default");
        }

        std::destroy(first, last);
    }

    TEST(Uninitialized, Fill)
    {
        constexpr int count{3};
        alignas(alignof(DefaultConstructable)) unsigned char memory[count * sizeof(DefaultConstructable)];

        auto first{reinterpret_cast<DefaultConstructable*>(memory)};
        auto last{first + count};

        orion::uninitialized_fill(first, last, DefaultConstructable{.value = "not-default"});

        for (auto it = first; it != last; ++it) {
            EXPECT_EQ(it->value, "not-default");
        }

        std::destroy(first, last);
    }

    TEST(Uninitialized, Copy)
    {
        const std::array<MoveCopyTest, 3> array;

        constexpr int count{3};
        alignas(alignof(MoveCopyTest)) unsigned char memory[count * sizeof(MoveCopyTest)];

        auto first{reinterpret_cast<MoveCopyTest*>(memory)};
        auto last{first + count};

        orion::uninitialized_copy(array.begin(), array.end(), first);

        for (auto it = first; it != last; ++it) {
            EXPECT_FALSE(it->default_ctor);
            EXPECT_TRUE(it->copy_ctor);
            EXPECT_FALSE(it->move_ctor);
        }

        std::destroy(first, last);
    }

    TEST(Uninitialized, Move)
    {
        std::array<MoveCopyTest, 3> array;

        constexpr int count{3};
        alignas(alignof(MoveCopyTest)) unsigned char memory[count * sizeof(MoveCopyTest)];

        auto first{reinterpret_cast<MoveCopyTest*>(memory)};
        auto last{first + count};

        orion::uninitialized_move(array.begin(), array.end(), first);

        for (auto it = first; it != last; ++it) {
            EXPECT_FALSE(it->default_ctor);
            EXPECT_FALSE(it->copy_ctor);
            EXPECT_TRUE(it->move_ctor);
        }

        std::destroy(first, last);
    }
} // namespace
