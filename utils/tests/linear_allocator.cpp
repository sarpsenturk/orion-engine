#include "orion-utils/allocators/linear_allocator.h"

#include <gtest/gtest.h>

namespace
{
    TEST(LinearAllocator, Ctor)
    {
        static constexpr auto max_size = 2048;
        const auto allocator = orion::LinearAllocator(max_size);
        EXPECT_EQ(allocator.max_available(), max_size);
        EXPECT_EQ(allocator.available(), max_size);
        EXPECT_FALSE(allocator.is_full());
    }

    TEST(LinearAllocator, Allocate)
    {
        static constexpr auto max_size = 2048;

        // Allocate default alignment
        {
            auto allocator = orion::LinearAllocator(max_size);
            const auto size = sizeof(int);
            const auto alignment = alignof(int);
            auto allocation = allocator.allocate(size, alignment);
            EXPECT_NE(allocation.ptr, nullptr);
            EXPECT_EQ(allocation.size, size);
            EXPECT_EQ((reinterpret_cast<std::uintptr_t>(allocation.ptr) % alignment), 0);
            EXPECT_FALSE(allocator.is_empty());
        }

        // Allocate too much
        {
            auto allocator = orion::LinearAllocator(0);
            EXPECT_THROW(allocator.allocate(sizeof(int), alignof(int)), std::bad_alloc);
        }
    }

    TEST(LinearAllocator, Deallocate)
    {
        auto allocator = orion::LinearAllocator(0);
        EXPECT_THROW(allocator.deallocate({}), std::logic_error);
    }

    TEST(LinearAllocator, Reset)
    {
        static constexpr auto max_size = 2048;

        auto allocator = orion::LinearAllocator(max_size);
        allocator.allocate(sizeof(int), alignof(int));
        ASSERT_FALSE(allocator.is_empty());

        allocator.reset();
        EXPECT_TRUE(allocator.is_empty());
    }
} // namespace
