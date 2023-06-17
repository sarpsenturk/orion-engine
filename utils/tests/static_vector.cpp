#include "orion-utils/static_vector.h"

#include <algorithm> // std::all_of
#include <gtest/gtest.h>

namespace
{
    struct DefaultConstructible {
        constexpr static int expected = 42;

        DefaultConstructible()
            : value(expected)
        {
        }

        int value;
    };

    struct NotDefaultConstructible {
        NotDefaultConstructible(int val)
            : value(val)
        {
        }

        int value;
    };

    TEST(StaticVector, DefaultCtor)
    {
        constexpr auto capacity = 5;
        const orion::static_vector<int, capacity> vector;
        EXPECT_EQ(vector.capacity(), capacity);
        EXPECT_EQ(vector.max_size(), capacity);
        EXPECT_EQ(vector.size(), 0);
        EXPECT_TRUE(vector.empty());
        EXPECT_EQ(std::distance(vector.begin(), vector.end()), 0);
    }

    TEST(StaticVector, DefaultNCtor)
    {
        constexpr auto capacity = 5;
        constexpr auto size = capacity - 1;
        const orion::static_vector<DefaultConstructible, capacity> vector(size);
        EXPECT_EQ(vector.capacity(), capacity);
        EXPECT_EQ(vector.max_size(), capacity);
        EXPECT_EQ(vector.size(), size);
        EXPECT_TRUE(std::all_of(vector.begin(), vector.end(), [](auto obj) { return obj.value == DefaultConstructible::expected; }));
        EXPECT_EQ(std::distance(vector.begin(), vector.end()), size);
    }

    TEST(StaticVector, FillNCtor)
    {
        constexpr auto capacity = 5;
        constexpr auto size = capacity - 2;
        constexpr auto expected = 42;
        const orion::static_vector<NotDefaultConstructible, capacity> vector(size, NotDefaultConstructible(expected));
        EXPECT_EQ(vector.capacity(), capacity);
        EXPECT_EQ(vector.max_size(), capacity);
        EXPECT_EQ(vector.size(), size);
        EXPECT_TRUE(std::all_of(vector.begin(), vector.end(), [](auto obj) { return obj.value == expected; }));
        EXPECT_EQ(std::distance(vector.begin(), vector.end()), size);
    }

    TEST(StaticVector, RangeCtor)
    {
        constexpr auto capacity = 5;
        constexpr auto size = capacity - 1;
        constexpr auto expected = 42;
        const std::vector<int> vector(size, expected);
        const orion::static_vector<int, capacity> static_vector(vector.begin(), vector.end());
        EXPECT_EQ(static_vector.capacity(), capacity);
        EXPECT_EQ(static_vector.max_size(), capacity);
        EXPECT_EQ(static_vector.size(), size);
        EXPECT_TRUE(std::equal(vector.begin(), vector.end(), static_vector.begin()));
        EXPECT_EQ(std::distance(vector.begin(), vector.end()), size);
    }

    TEST(StaticVector, CopyCtor)
    {
        constexpr auto expected = 42;
        orion::static_vector<int, 3> vector(2, expected);

        const auto copy(vector);
        EXPECT_TRUE(std::equal(vector.begin(), vector.end(), copy.begin()));
    }

    TEST(StaticVector, MoveCtor)
    {
        constexpr auto size = 3;
        constexpr auto expected = 42;
        const auto vector(orion::static_vector<int, size>(size, expected));
        EXPECT_EQ(vector.capacity(), size);
        EXPECT_EQ(vector.size(), size);
    }

    TEST(StaticVector, Emplace)
    {
        constexpr auto expected = 42;
        orion::static_vector<int, 3> vector;
        ASSERT_EQ(vector.size(), 0);

        vector.emplace(vector.begin(), expected);
        EXPECT_EQ(vector.size(), 1);
        EXPECT_EQ(*vector.begin(), expected);
    }

    TEST(StaticVector, EmplaceBack)
    {
        constexpr auto expected = 42;
        orion::static_vector<int, 3> vector(1, 0);
        ASSERT_EQ(vector.size(), 1);

        vector.emplace_back(expected);
        EXPECT_EQ(vector.size(), 2);
        EXPECT_EQ(vector.back(), expected);
    }

    TEST(StaticVector, PushBack)
    {
        constexpr auto expected = 42;
        orion::static_vector<int, 3> vector(1, 0);
        ASSERT_EQ(vector.size(), 1);

        vector.push_back(expected);
        EXPECT_EQ(vector.size(), 2);
        EXPECT_EQ(vector.back(), expected);
    }

    TEST(StaticVector, Insert)
    {
        constexpr auto expected = 42;
        orion::static_vector<int, 5> vector(2, 5);
        ASSERT_EQ(vector.size(), 2);

        auto iter = vector.insert(vector.begin() + 1, expected);
        EXPECT_EQ(*iter, expected);
        EXPECT_EQ(vector.size(), 3);
        EXPECT_NE(iter, vector.begin());
        EXPECT_NE(iter, vector.end());
        EXPECT_EQ(vector[0], 5);
        EXPECT_EQ(vector[1], expected);
        EXPECT_EQ(vector[2], 5);
    }

    TEST(StaticVector, InsertRange)
    {
        const std::vector vector{1, 2, 3};
        orion::static_vector<int, 3> static_vector;
        static_vector.insert(static_vector.begin(), vector.begin(), vector.end());
        EXPECT_TRUE(std::equal(static_vector.begin(), static_vector.end(), vector.begin()));
    }

    TEST(StaticVector, InsertInitializerList)
    {
        const std::array expected{1, 2, 3};
        orion::static_vector<int, 3> vector;
        vector.insert(vector.begin(), {1, 2, 3});
        EXPECT_TRUE(std::equal(vector.begin(), vector.end(), expected.begin()));
    }

    TEST(StaticVector, Erase)
    {
        const std::array expected{3, 1};

        orion::static_vector<int, 3> vector;
        vector.insert(vector.begin(), 1);
        vector.insert(vector.begin(), 2);
        vector.insert(vector.begin(), 3);
        ASSERT_EQ(vector.size(), 3);

        auto iter = vector.erase(vector.begin() + 1);
        EXPECT_EQ(vector.size(), 2);
        EXPECT_TRUE(std::equal(vector.begin(), vector.end(), expected.begin()));
        EXPECT_EQ(*iter, 1);
    }

    TEST(StaticVector, EraseRange)
    {
        const std::array expected{1, 5};
        orion::static_vector<int, 5> vector;
        vector.insert(vector.begin(), {1, 2, 3, 4, 5});
        ASSERT_EQ(vector.size(), 5);

        auto iter = vector.erase(vector.begin() + 1, vector.begin() + 4);
        EXPECT_EQ(vector.size(), 2);
        EXPECT_TRUE(std::equal(vector.begin(), vector.end(), expected.begin()));
        EXPECT_EQ(*iter, 5);
    }

    TEST(StaticVector, PopBack)
    {
        const std::array expected{1, 2};
        orion::static_vector<int, 3> vector;
        vector.insert(vector.begin(), {1, 2, 3});
        ASSERT_EQ(vector.size(), 3);

        vector.pop_back();
        EXPECT_EQ(vector.size(), 2);
        EXPECT_TRUE(std::equal(vector.begin(), vector.end(), expected.begin()));
    }

    TEST(StaticVector, Comparison)
    {
        {
            const orion::static_vector<int, 3> lhs(3, 1);
            const orion::static_vector<int, 3> rhs(3, 1);
            EXPECT_EQ(lhs, rhs);
        }

        {
            const orion::static_vector<int, 3> lhs(3, 1);
            const orion::static_vector<int, 3> rhs(3, 2);
            EXPECT_NE(lhs, rhs);
        }
    }

    TEST(StaticVector, Constexpr)
    {
        constexpr auto result = []() {
            orion::static_vector<int, 3> vector;
            vector.insert(vector.begin(), {1, 2, 3});
            vector.clear();
            return vector.size();
        }();
        static_assert(result == 0);
    }
} // namespace