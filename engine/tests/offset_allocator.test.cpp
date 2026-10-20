#include "orion/memory/offset_allocator.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <limits>
#include <random>
#include <vector>

namespace orion
{
    namespace
    {
        TEST(OffsetAllocator, EmptyAndZeroSizedRequests)
        {
            OffsetAllocator empty(0);
            EXPECT_FALSE(empty.allocate(1));
            EXPECT_EQ(empty.free_size(), 0u);
            OffsetAllocator allocator(1'024);
            EXPECT_FALSE(allocator.allocate(0));
            EXPECT_FALSE(allocator.allocate(1'025));
            EXPECT_EQ(allocator.free_size(), 1'024u);
            OffsetAllocator no_metadata(1'024, 0);
            EXPECT_FALSE(no_metadata.allocate(1));
        }

        TEST(OffsetAllocator, ExactFitAndExhaustion)
        {
            OffsetAllocator allocator(1'003, 1);
            const auto allocation = allocator.allocate(1'003);
            ASSERT_TRUE(allocation);
            EXPECT_EQ(allocation.offset, 0u);
            EXPECT_EQ(allocation.size, 1'003u);
            EXPECT_EQ(allocator.capacity(), 1'003u);
            EXPECT_EQ(allocator.free_size(), 0u);
            EXPECT_FALSE(allocator.allocate(1));
            allocator.deallocate(allocation);
            EXPECT_EQ(allocator.free_size(), 1'003u);
            EXPECT_TRUE(allocator.allocate(1'003));
        }

        TEST(OffsetAllocator, MetadataExhaustionDoesNotMutateFreeRanges)
        {
            OffsetAllocator allocator(100, 2);
            const auto first = allocator.allocate(40);
            ASSERT_TRUE(first);
            EXPECT_FALSE(allocator.allocate(30));
            EXPECT_EQ(allocator.free_size(), 60u);
            const auto second = allocator.allocate(60);
            ASSERT_TRUE(second);
            EXPECT_EQ(second.offset, 40u);
            allocator.deallocate(first);
            allocator.deallocate(second);
            const auto split = allocator.allocate(1);
            ASSERT_TRUE(split);
            allocator.deallocate(split);
            EXPECT_TRUE(allocator.allocate(100));
        }

        TEST(OffsetAllocator, CoalescesInEveryFreeOrder)
        {
            std::array<std::uint32_t, 4> order{0, 1, 2, 3};
            do {
                OffsetAllocator allocator(256, 4);
                std::array<OffsetAllocator::Allocation, 4> allocations;
                for (auto& allocation : allocations) {
                    allocation = allocator.allocate(64);
                    ASSERT_TRUE(allocation);
                }
                for (const auto index : order) allocator.deallocate(allocations[index]);
                EXPECT_EQ(allocator.free_size(), 256u);
                const auto whole = allocator.allocate(256);
                ASSERT_TRUE(whole);
                EXPECT_EQ(whole.offset, 0u);
            } while (std::next_permutation(order.begin(), order.end()));
        }

        TEST(OffsetAllocator, FragmentedSpaceCannotSatisfyContiguousRequest)
        {
            OffsetAllocator allocator(256, 4);
            const auto a = allocator.allocate(64);
            const auto b = allocator.allocate(64);
            const auto c = allocator.allocate(64);
            const auto d = allocator.allocate(64);
            ASSERT_TRUE(a);
            ASSERT_TRUE(b);
            ASSERT_TRUE(c);
            ASSERT_TRUE(d);
            allocator.deallocate(a);
            allocator.deallocate(c);
            EXPECT_EQ(allocator.free_size(), 128u);
            EXPECT_FALSE(allocator.allocate(128));
            allocator.deallocate(b);
            const auto merged = allocator.allocate(192);
            ASSERT_TRUE(merged);
            EXPECT_EQ(merged.offset, 0u);
            allocator.deallocate(d);
            allocator.deallocate(merged);
            EXPECT_TRUE(allocator.allocate(256));
        }

        TEST(OffsetAllocator, AlignmentPaddingIsReclaimed)
        {
            OffsetAllocator allocator(256, 3);
            const auto first   = allocator.allocate(3);
            const auto aligned = allocator.allocate(17, 64);
            ASSERT_TRUE(first);
            ASSERT_TRUE(aligned);
            EXPECT_EQ(aligned.offset, 64u);
            EXPECT_EQ(aligned.size, 17u);
            EXPECT_EQ(allocator.free_size(), 175u);
            allocator.deallocate(aligned);
            EXPECT_EQ(allocator.free_size(), 253u);
            allocator.deallocate(first);
            EXPECT_TRUE(allocator.allocate(256, 256));
        }

        TEST(OffsetAllocator, AlignmentSearchSkipsInsufficientRange)
        {
            OffsetAllocator allocator(256, 8);
            const auto first     = allocator.allocate(1);
            const auto hole      = allocator.allocate(32);
            const auto separator = allocator.allocate(1);
            ASSERT_TRUE(first);
            ASSERT_TRUE(hole);
            ASSERT_TRUE(separator);
            allocator.deallocate(hole);
            const auto aligned = allocator.allocate(32, 64);
            ASSERT_TRUE(aligned);
            EXPECT_EQ(aligned.offset, 64u);
            allocator.deallocate(aligned);
            allocator.deallocate(separator);
            allocator.deallocate(first);
            EXPECT_TRUE(allocator.allocate(256));
        }

        TEST(OffsetAllocator, ResetRestoresMetadataAndCapacity)
        {
            OffsetAllocator allocator(128, 2);
            ASSERT_TRUE(allocator.allocate(64));
            ASSERT_TRUE(allocator.allocate(64));
            allocator.reset();
            EXPECT_EQ(allocator.free_size(), 128u);
            const auto first = allocator.allocate(32);
            ASSERT_TRUE(first);
            EXPECT_EQ(first.offset, 0u);
            EXPECT_TRUE(allocator.allocate(96));
        }

        TEST(OffsetAllocator, SizeClassBoundaries)
        {
            for (std::uint32_t exponent = 3; exponent < 32; ++exponent) {
                const auto base = std::uint32_t{1} << exponent;
                const auto step = base / 8;
                for (std::uint32_t subdivision = 0; subdivision < 8; ++subdivision) {
                    const auto boundary = base + step * subdivision;
                    for (const auto size : {boundary - 1, boundary, boundary + 1}) {
                        OffsetAllocator allocator(size, 2);
                        const auto exact = allocator.allocate(size);
                        ASSERT_TRUE(exact) << size;
                        allocator.deallocate(exact);
                        const auto partial = allocator.allocate(size - 1);
                        ASSERT_TRUE(partial) << size;
                        const auto last = allocator.allocate(1);
                        ASSERT_TRUE(last) << size;
                        EXPECT_EQ(last.offset, size - 1);
                        allocator.deallocate(partial);
                        allocator.deallocate(last);
                        EXPECT_TRUE(allocator.allocate(size)) << size;
                    }
                }
            }
        }

        TEST(OffsetAllocator, MaximumCapacityAndAlignmentArithmetic)
        {
            constexpr auto maximum = std::numeric_limits<std::uint32_t>::max();
            OffsetAllocator allocator(maximum, 4);
            const auto first = allocator.allocate(1);
            ASSERT_TRUE(first);
            EXPECT_FALSE(allocator.allocate(maximum - 1, 4));
            EXPECT_EQ(allocator.free_size(), maximum - 1);
            const auto aligned = allocator.allocate(1, std::uint32_t{1} << 31);
            ASSERT_TRUE(aligned);
            EXPECT_EQ(aligned.offset, std::uint32_t{1} << 31);
            allocator.deallocate(first);
            allocator.deallocate(aligned);
            const auto whole = allocator.allocate(maximum);
            ASSERT_TRUE(whole);
            EXPECT_EQ(whole.offset, 0u);
        }

        TEST(OffsetAllocator, RandomizedAllocationsNeverOverlapAndRecoverAllSpace)
        {
            constexpr std::uint32_t capacity = 16'384;
            OffsetAllocator allocator(capacity, 256);
            std::mt19937 random(12'345);
            std::vector<OffsetAllocator::Allocation> live;
            std::vector<bool> occupied(capacity, false);
            for (std::uint32_t iteration = 0; iteration < 20'000; ++iteration) {
                if (!live.empty() && random() % 3 == 0) {
                    const auto index      = random() % live.size();
                    const auto allocation = live[index];
                    for (std::uint32_t i = allocation.offset; i < allocation.offset + allocation.size; ++i) occupied[i] = false;
                    allocator.deallocate(allocation);
                    live[index] = live.back();
                    live.pop_back();
                } else {
                    const auto size       = static_cast<std::uint32_t>(random() % 512 + 1);
                    const auto alignment  = std::uint32_t{1} << (random() % 8);
                    const auto allocation = allocator.allocate(size, alignment);
                    if (!allocation) continue;
                    EXPECT_EQ(allocation.offset % alignment, 0u);
                    ASSERT_LE(static_cast<std::uint64_t>(allocation.offset) + size, capacity);
                    EXPECT_EQ(allocation.size, size);
                    for (std::uint32_t i = allocation.offset; i < allocation.offset + size; ++i) {
                        ASSERT_FALSE(occupied[i]);
                        occupied[i] = true;
                    }
                    live.push_back(allocation);
                }
            }
            for (const auto allocation : live) allocator.deallocate(allocation);
            EXPECT_EQ(allocator.free_size(), capacity);
            EXPECT_TRUE(allocator.allocate(capacity));
        }
    } // namespace
} // namespace orion
