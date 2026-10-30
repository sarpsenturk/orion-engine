#include "orion/utils/slot_map.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace orion
{
    namespace
    {
        using IntSlotMap = SlotMap<int>;
        using Handle     = IntSlotMap::Handle;

        // Accessors expose the stored value and preserve the map's constness.
        static_assert(std::is_same_v<decltype(std::declval<IntSlotMap&>().get(Handle{})), int&>);
        static_assert(std::is_same_v<decltype(std::declval<const IntSlotMap&>().get(Handle{})), const int&>);
        static_assert(std::is_same_v<decltype(std::declval<IntSlotMap&>().try_get(Handle{})), int*>);
        static_assert(std::is_same_v<decltype(std::declval<const IntSlotMap&>().try_get(Handle{})), const int*>);
        static_assert(std::is_same_v<decltype(std::declval<IntSlotMap&&>().get(Handle{})), int&&>);
        static_assert(std::is_same_v<decltype(std::declval<const IntSlotMap&&>().get(Handle{})), const int&&>);
        static_assert(std::is_same_v<decltype(*std::declval<IntSlotMap&>().begin()), int&>);
        static_assert(std::is_same_v<decltype(*std::declval<const IntSlotMap&>().begin()), const int&>);
        static_assert(std::is_same_v<decltype(*std::declval<IntSlotMap&>().cbegin()), const int&>);

        // Iteration order is unspecified; compare contents without imposing one.
        std::vector<int> sorted_values(const IntSlotMap& map)
        {
            std::vector<int> values;
            for (const auto& value : map) {
                values.push_back(value);
            }
            std::sort(values.begin(), values.end());
            return values;
        }

        TEST(SlotMapTest, EmptyMapHasEmptyIteratorRanges)
        {
            IntSlotMap map;
            const auto& const_map = std::as_const(map);

            EXPECT_TRUE(map.empty());
            EXPECT_EQ(map.size(), 0u);
            EXPECT_EQ(map.begin(), map.end());
            EXPECT_EQ(const_map.begin(), const_map.end());
            EXPECT_EQ(map.cbegin(), map.cend());
            EXPECT_EQ(const_map.cbegin(), const_map.cend());
            EXPECT_TRUE(sorted_values(map).empty());
        }

        TEST(SlotMapTest, SizeTracksLiveValuesThroughErasureAndReuse)
        {
            IntSlotMap map;
            const auto first  = map.insert(10);
            const auto second = map.insert(20);
            EXPECT_FALSE(map.empty());
            EXPECT_EQ(map.size(), 2u);

            map.erase(first);
            map.erase(first);
            map.erase({});
            EXPECT_EQ(map.size(), 1u);
            EXPECT_EQ(sorted_values(map), std::vector<int>{20});

            map.erase(second);
            EXPECT_TRUE(map.empty());
            EXPECT_EQ(map.size(), 0u);
            EXPECT_EQ(map.begin(), map.end());
            EXPECT_EQ(map.cbegin(), map.cend());

            const auto reused = map.insert(30);
            ASSERT_TRUE(map.contains(reused));
            EXPECT_FALSE(map.empty());
            EXPECT_EQ(map.size(), 1u);
            EXPECT_EQ(sorted_values(map), std::vector<int>{30});
        }

        TEST(SlotMapTest, IterationVisitsOnlyLiveValuesIncludingDuplicates)
        {
            IntSlotMap map;
            const auto first = map.insert(10);
            map.insert(20);
            const auto middle = map.insert(30);
            map.insert(20);
            const auto last = map.insert(40);
            map.erase(first);
            map.erase(middle);
            map.erase(last);
            map.insert(50);

            EXPECT_EQ(map.size(), 3u);
            EXPECT_EQ(sorted_values(map), (std::vector<int>{20, 20, 50}));
            const auto& const_map = std::as_const(map);
            std::vector<int> values(const_map.cbegin(), const_map.cend());
            std::sort(values.begin(), values.end());
            EXPECT_EQ(values, (std::vector<int>{20, 20, 50}));
        }

        TEST(SlotMapTest, MutableIterationUpdatesValuesAccessibleByHandle)
        {
            IntSlotMap map;
            const auto first  = map.insert(10);
            const auto erased = map.insert(20);
            const auto last   = map.insert(30);
            map.erase(erased);

            for (auto& value : map) {
                value += 5;
            }

            EXPECT_EQ(map.get(first), 15);
            EXPECT_EQ(map.get(last), 35);
            EXPECT_EQ(sorted_values(map), (std::vector<int>{15, 35}));
        }

        TEST(SlotMapTest, RepeatedCompactionPreservesHandleLookupsAndIteration)
        {
            IntSlotMap map;
            std::vector<Handle> handles;
            constexpr int count = 64;
            for (int value = 0; value < count; ++value) {
                handles.push_back(map.insert(value));
            }

            // This visits every handle once, repeatedly erasing relocated values.
            std::vector<bool> alive(count, true);
            for (int step = 0; step < count; ++step) {
                SCOPED_TRACE(step);
                const auto index = static_cast<std::size_t>((step * 17) % count);
                map.erase(handles[index]);
                alive[index] = false;

                std::vector<int> expected;
                for (int value = 0; value < count; ++value) {
                    const auto i = static_cast<std::size_t>(value);
                    if (alive[i]) {
                        ASSERT_TRUE(map.contains(handles[i]));
                        ASSERT_NE(map.try_get(handles[i]), nullptr);
                        EXPECT_EQ(map.get(handles[i]), value);
                        EXPECT_EQ(*map.try_get(handles[i]), value);
                        expected.push_back(value);
                    } else {
                        EXPECT_FALSE(map.contains(handles[i]));
                        EXPECT_EQ(map.try_get(handles[i]), nullptr);
                    }
                }
                EXPECT_EQ(map.size(), expected.size());
                EXPECT_EQ(sorted_values(map), expected);
            }
            EXPECT_TRUE(map.empty());
        }

        TEST(SlotMapTest, CopyConstructionPreservesHandlesAndIndependentStorage)
        {
            IntSlotMap source;
            const auto erased = source.insert(10);
            const auto live   = source.insert(20);
            source.erase(erased);

            IntSlotMap copy(source);
            ASSERT_TRUE(copy.contains(live));
            EXPECT_FALSE(copy.contains(erased));
            EXPECT_EQ(sorted_values(copy), std::vector<int>{20});
            copy.get(live) = 25;
            EXPECT_EQ(source.get(live), 20);

            const auto replacement = copy.insert(30);
            EXPECT_EQ(replacement.index, erased.index);
            EXPECT_NE(replacement.generation, erased.generation);
            EXPECT_FALSE(copy.contains(erased));
            EXPECT_FALSE(source.contains(replacement));
            source.erase(live);
            EXPECT_EQ(sorted_values(copy), (std::vector<int>{25, 30}));
            EXPECT_TRUE(source.empty());
        }

        TEST(SlotMapTest, CopyAssignmentReplacesContentsAndPreservesFreeSlots)
        {
            IntSlotMap source;
            const auto erased = source.insert(10);
            const auto live   = source.insert(20);
            source.erase(erased);
            IntSlotMap destination;
            destination.insert(100);
            destination.insert(200);
            destination.insert(300);

            destination = source;

            ASSERT_TRUE(destination.contains(live));
            EXPECT_FALSE(destination.contains(erased));
            EXPECT_EQ(destination.size(), 1u);
            EXPECT_EQ(sorted_values(destination), std::vector<int>{20});
            const auto reused = destination.insert(30);
            EXPECT_EQ(reused.index, erased.index);
            EXPECT_NE(reused.generation, erased.generation);
            destination.get(live) = 25;
            EXPECT_EQ(sorted_values(source), std::vector<int>{20});
            EXPECT_EQ(sorted_values(destination), (std::vector<int>{25, 30}));
        }

        TEST(SlotMapTest, MoveConstructionTransfersHandlesAndLeavesSourceReusable)
        {
            IntSlotMap source;
            const auto erased = source.insert(10);
            const auto live   = source.insert(20);
            source.erase(erased);

            IntSlotMap destination(std::move(source));

            EXPECT_TRUE(source.empty());
            EXPECT_EQ(source.size(), 0u);
            EXPECT_EQ(source.begin(), source.end());
            EXPECT_FALSE(source.contains(live));
            ASSERT_TRUE(destination.contains(live));
            EXPECT_FALSE(destination.contains(erased));
            EXPECT_EQ(sorted_values(destination), std::vector<int>{20});
            const auto reused = destination.insert(30);
            EXPECT_EQ(reused.index, erased.index);
            EXPECT_NE(reused.generation, erased.generation);

            const auto fresh = source.insert(40);
            EXPECT_EQ(source.get(fresh), 40);
            EXPECT_EQ(sorted_values(destination), (std::vector<int>{20, 30}));
        }

        TEST(SlotMapTest, MoveAssignmentReleasesOldValuesAndLeavesSourceReusable)
        {
            SlotMap<std::shared_ptr<int>> source;
            const auto retained = std::make_shared<int>(20);
            const auto erased   = source.insert(retained);
            const auto live     = source.insert(retained);
            source.erase(erased);
            SlotMap<std::shared_ptr<int>> destination;
            const auto replaced = std::make_shared<int>(100);
            destination.insert(replaced);
            ASSERT_EQ(replaced.use_count(), 2);

            destination = std::move(source);

            EXPECT_EQ(replaced.use_count(), 1);
            EXPECT_EQ(retained.use_count(), 2);
            EXPECT_TRUE(source.empty());
            EXPECT_EQ(source.begin(), source.end());
            EXPECT_FALSE(source.contains(live));
            ASSERT_TRUE(destination.contains(live));
            EXPECT_EQ(destination.get(live), retained);
            EXPECT_FALSE(destination.contains(erased));
            const auto reused = destination.insert(retained);
            EXPECT_EQ(reused.index, erased.index);
            EXPECT_NE(reused.generation, erased.generation);
            const auto fresh = source.insert(replaced);
            EXPECT_EQ(source.get(fresh), replaced);
            EXPECT_EQ(destination.size(), 2u);
        }

        TEST(SlotMapTest, SelfAssignmentPreservesValuesAndFreeSlots)
        {
            IntSlotMap map;
            const auto erased = map.insert(10);
            const auto live   = map.insert(20);
            map.erase(erased);
            auto& alias = map;

            map = alias;
            map = std::move(alias);

            ASSERT_TRUE(map.contains(live));
            EXPECT_FALSE(map.contains(erased));
            EXPECT_EQ(sorted_values(map), std::vector<int>{20});
            const auto reused = map.insert(30);
            EXPECT_EQ(reused.index, erased.index);
            EXPECT_NE(reused.generation, erased.generation);
            EXPECT_EQ(sorted_values(map), (std::vector<int>{20, 30}));
        }

        TEST(SlotMapTest, SwapTransfersHandlesValuesAndFreeSlots)
        {
            IntSlotMap first;
            const auto erased = first.insert(10);
            const auto live   = first.insert(20);
            first.erase(erased);
            IntSlotMap second;
            const auto other = second.insert(30);

            first.swap(second);

            ASSERT_TRUE(first.contains(other));
            ASSERT_TRUE(second.contains(live));
            EXPECT_FALSE(second.contains(erased));
            EXPECT_EQ(sorted_values(first), std::vector<int>{30});
            EXPECT_EQ(sorted_values(second), std::vector<int>{20});
            const auto reused = second.insert(40);
            EXPECT_EQ(reused.index, erased.index);
            EXPECT_NE(reused.generation, erased.generation);

            IntSlotMap empty;
            second.swap(empty);
            EXPECT_TRUE(second.empty());
            EXPECT_EQ(second.begin(), second.end());
            EXPECT_EQ(sorted_values(empty), (std::vector<int>{20, 40}));
            ASSERT_TRUE(empty.contains(live));
            ASSERT_TRUE(empty.contains(reused));
            empty.erase(live);
            EXPECT_EQ(empty.get(reused), 40);
            const auto fresh = second.insert(50);
            EXPECT_EQ(second.get(fresh), 50);
        }

        TEST(SlotMapTest, DefaultHandleIsInvalid)
        {
            const Handle handle;
            const IntSlotMap map;

            EXPECT_EQ(handle.index, IntSlotMap::invalid_index);
            EXPECT_EQ(handle.generation, 0u);
            EXPECT_FALSE(map.contains(handle));
            EXPECT_EQ(map.try_get(handle), nullptr);
        }

        TEST(SlotMapTest, HandleEqualityComparesIndexAndGeneration)
        {
            const Handle handle{2, 3};
            const Handle equal{2, 3};
            const Handle different_index{1, 3};
            const Handle different_generation{2, 4};

            EXPECT_EQ(handle, equal);
            EXPECT_NE(handle, different_index);
            EXPECT_NE(handle, different_generation);
        }

        TEST(SlotMapTest, EmptyMapRejectsHandles)
        {
            IntSlotMap map;
            const Handle handles[] = {{}, {0, 0}, {42, 1}, {IntSlotMap::invalid_index, 1}};

            for (const auto handle : handles) {
                EXPECT_FALSE(map.contains(handle));
                EXPECT_EQ(map.try_get(handle), nullptr);
                EXPECT_NO_THROW(map.erase(handle));
            }

            const auto inserted = map.insert(10);
            ASSERT_TRUE(map.contains(inserted));
            EXPECT_EQ(map.get(inserted), 10);
        }

        TEST(SlotMapTest, InsertReturnsHandleToStoredValue)
        {
            IntSlotMap map;
            const auto handle = map.insert(42);

            EXPECT_NE(handle.index, IntSlotMap::invalid_index);
            ASSERT_TRUE(map.contains(handle));
            EXPECT_EQ(map.get(handle), 42);
            ASSERT_NE(map.try_get(handle), nullptr);
            EXPECT_EQ(*map.try_get(handle), 42);
            EXPECT_EQ(map.try_get(handle), &map.get(handle));
        }

        TEST(SlotMapTest, EqualValuesHaveIndependentHandles)
        {
            IntSlotMap map;
            const auto first  = map.insert(7);
            const auto second = map.insert(7);

            EXPECT_NE(first.index, second.index);
            map.get(first) = 8;
            EXPECT_EQ(map.get(first), 8);
            EXPECT_EQ(map.get(second), 7);
        }

        TEST(SlotMapTest, InsertCopiesTheSourceValue)
        {
            SlotMap<std::string> map;
            std::string source = "original";
            const auto handle  = map.insert(source);

            source = "changed";

            EXPECT_EQ(map.get(handle), "original");
        }

        TEST(SlotMapTest, MutableAccessorsModifyTheStoredValue)
        {
            IntSlotMap map;
            const auto handle = map.insert(10);

            map.get(handle) = 20;
            ASSERT_NE(map.try_get(handle), nullptr);
            EXPECT_EQ(*map.try_get(handle), 20);

            *map.try_get(handle) = 30;
            EXPECT_EQ(map.get(handle), 30);
        }

        TEST(SlotMapTest, ConstAccessorsReadTheStoredValue)
        {
            IntSlotMap map;
            const auto handle     = map.insert(42);
            const auto& const_map = std::as_const(map);

            EXPECT_TRUE(const_map.contains(handle));
            EXPECT_EQ(const_map.get(handle), 42);
            ASSERT_NE(const_map.try_get(handle), nullptr);
            EXPECT_EQ(*const_map.try_get(handle), 42);
            EXPECT_EQ(const_map.try_get(handle), &const_map.get(handle));
        }

        TEST(SlotMapTest, InvalidHandlesDoNotAffectLiveValues)
        {
            IntSlotMap map;
            const auto live        = map.insert(42);
            const Handle handles[] = {
                {},
                {live.index + 1, live.generation},
                {IntSlotMap::invalid_index, live.generation},
                {live.index, live.generation + 1},
            };

            for (const auto handle : handles) {
                EXPECT_FALSE(map.contains(handle));
                EXPECT_EQ(map.try_get(handle), nullptr);
                EXPECT_EQ(std::as_const(map).try_get(handle), nullptr);
                map.erase(handle);
                ASSERT_TRUE(map.contains(live));
                EXPECT_EQ(map.get(live), 42);
            }
        }

        TEST(SlotMapTest, EraseInvalidatesTheHandle)
        {
            IntSlotMap map;
            const auto handle = map.insert(42);

            map.erase(handle);

            EXPECT_FALSE(map.contains(handle));
            EXPECT_EQ(map.try_get(handle), nullptr);
            EXPECT_EQ(std::as_const(map).try_get(handle), nullptr);
        }

        TEST(SlotMapTest, VacantSlotRejectsItsCurrentGeneration)
        {
            IntSlotMap map;
            const auto handle = map.insert(42);
            map.erase(handle);

            // Matching a vacant slot's generation must not make it a live value.
            const Handle vacant{handle.index, handle.generation + 1};
            EXPECT_FALSE(map.contains(vacant));
            EXPECT_EQ(map.try_get(vacant), nullptr);
            EXPECT_EQ(std::as_const(map).try_get(vacant), nullptr);
        }

        TEST(SlotMapTest, ErasePreservesOtherValues)
        {
            IntSlotMap map;
            const auto first  = map.insert(10);
            const auto middle = map.insert(20);
            const auto last   = map.insert(30);

            map.erase(middle);

            ASSERT_TRUE(map.contains(first));
            ASSERT_TRUE(map.contains(last));
            EXPECT_EQ(map.get(first), 10);
            EXPECT_EQ(map.get(last), 30);
            EXPECT_FALSE(map.contains(middle));
        }

        TEST(SlotMapTest, InsertReusesErasedSlotWithANewGeneration)
        {
            IntSlotMap map;
            const auto erased = map.insert(10);
            map.erase(erased);

            const auto replacement = map.insert(20);

            EXPECT_EQ(replacement.index, erased.index);
            EXPECT_NE(replacement.generation, erased.generation);
            EXPECT_FALSE(map.contains(erased));
            EXPECT_EQ(map.try_get(erased), nullptr);
            ASSERT_TRUE(map.contains(replacement));
            EXPECT_EQ(map.get(replacement), 20);
        }

        TEST(SlotMapTest, ErasingStaleHandleDoesNotEraseReplacement)
        {
            IntSlotMap map;
            const auto stale = map.insert(10);
            map.erase(stale);
            const auto replacement = map.insert(20);

            map.erase(stale);

            ASSERT_TRUE(map.contains(replacement));
            EXPECT_EQ(map.get(replacement), 20);
            const auto another = map.insert(30);
            EXPECT_NE(another.index, replacement.index);
            EXPECT_EQ(map.get(replacement), 20);
        }

        TEST(SlotMapTest, RepeatedEraseDoesNotDuplicateFreeSlots)
        {
            IntSlotMap map;
            const auto erased = map.insert(10);

            map.erase(erased);
            map.erase(erased);
            map.erase(erased);

            const auto first  = map.insert(20);
            const auto second = map.insert(30);
            EXPECT_EQ(first.index, erased.index);
            EXPECT_NE(first.index, second.index);
            EXPECT_EQ(map.get(first), 20);
            EXPECT_EQ(map.get(second), 30);
        }

        TEST(SlotMapTest, ReusesAllFreeSlotsWithoutOverwritingLiveValues)
        {
            IntSlotMap map;
            const auto first = map.insert(10);
            const auto live  = map.insert(20);
            const auto last  = map.insert(30);
            map.erase(first);
            map.erase(last);

            const auto reused_first  = map.insert(40);
            const auto reused_second = map.insert(50);
            const auto appended      = map.insert(60);

            // Reuse order is deliberately unspecified.
            EXPECT_TRUE(reused_first.index == first.index || reused_first.index == last.index);
            EXPECT_TRUE(reused_second.index == first.index || reused_second.index == last.index);
            EXPECT_NE(reused_first.index, reused_second.index);
            EXPECT_NE(appended.index, reused_first.index);
            EXPECT_NE(appended.index, reused_second.index);
            EXPECT_NE(appended.index, live.index);
            EXPECT_FALSE(map.contains(first));
            EXPECT_FALSE(map.contains(last));
            EXPECT_EQ(map.get(live), 20);
            EXPECT_EQ(map.get(reused_first), 40);
            EXPECT_EQ(map.get(reused_second), 50);
            EXPECT_EQ(map.get(appended), 60);
        }

        TEST(SlotMapTest, RepeatedReuseNeverRevivesOldHandles)
        {
            IntSlotMap map;
            std::vector<Handle> stale_handles;
            auto current = map.insert(0);

            for (int value = 1; value <= 64; ++value) {
                SCOPED_TRACE(value);
                stale_handles.push_back(current);
                map.erase(current);
                const auto replacement = map.insert(value);
                EXPECT_EQ(replacement.index, current.index);
                EXPECT_NE(replacement.generation, current.generation);
                current = replacement;

                for (const auto stale : stale_handles) {
                    EXPECT_FALSE(map.contains(stale));
                    EXPECT_EQ(map.try_get(stale), nullptr);
                    map.erase(stale);
                }
                ASSERT_TRUE(map.contains(current));
                EXPECT_EQ(map.get(current), value);
            }
        }

        TEST(SlotMapTest, HandlesRemainValidAcrossStorageGrowth)
        {
            IntSlotMap map;
            std::vector<Handle> handles;
            constexpr int count = 1'024;

            for (int value = 0; value < count; ++value) {
                handles.push_back(map.insert(value));
            }

            for (int value = 0; value < count; ++value) {
                SCOPED_TRACE(value);
                const auto handle = handles[static_cast<std::size_t>(value)];
                ASSERT_TRUE(map.contains(handle));
                EXPECT_EQ(map.get(handle), value);
            }
        }

        struct NonDefaultConstructible {
            explicit NonDefaultConstructible(int initial_value)
                : value(initial_value)
            {
            }

            int value;
        };

        TEST(SlotMapTest, SupportsNonDefaultConstructibleValues)
        {
            SlotMap<NonDefaultConstructible> map;
            const auto handle = map.insert(NonDefaultConstructible{42});

            EXPECT_EQ(map.get(handle).value, 42);
            map.erase(handle);
            const auto replacement = map.insert(NonDefaultConstructible{99});
            EXPECT_EQ(map.get(replacement).value, 99);
        }

        struct NonAssignable {
            const int value;
        };

        static_assert(!std::is_move_assignable_v<NonAssignable>);
        static_assert(std::is_nothrow_move_constructible_v<NonAssignable>);

        TEST(SlotMapTest, EraseRelocatesNonAssignableValues)
        {
            SlotMap<NonAssignable> map;
            const auto first  = map.insert(NonAssignable{10});
            const auto middle = map.insert(NonAssignable{20});
            const auto last   = map.insert(NonAssignable{30});

            map.erase(first);
            ASSERT_TRUE(map.contains(last));
            EXPECT_EQ(map.get(last).value, 30);
            map.erase(last);
            ASSERT_TRUE(map.contains(middle));
            EXPECT_EQ(map.get(middle).value, 20);
            ASSERT_EQ(map.size(), 1u);
            EXPECT_EQ(map.begin()->value, 20);
            map.erase(middle);
            EXPECT_TRUE(map.empty());
        }

        struct ThrowOnMove {
            explicit ThrowOnMove(int initial_value)
                : value(initial_value)
            {
            }

            ThrowOnMove(const ThrowOnMove&) noexcept = default;
            ThrowOnMove(ThrowOnMove&&) { throw std::runtime_error("move must not be used"); }

            int value;
        };

        TEST(SlotMapTest, EraseUsesNonthrowingCopyWhenMoveCanThrow)
        {
            SlotMap<ThrowOnMove> map;
            const auto first = map.insert(ThrowOnMove{10});
            const auto last  = map.insert(ThrowOnMove{20});

            EXPECT_NO_THROW(map.erase(first));

            EXPECT_FALSE(map.contains(first));
            ASSERT_TRUE(map.contains(last));
            EXPECT_EQ(map.get(last).value, 20);
            ASSERT_EQ(map.size(), 1u);
            EXPECT_EQ(map.begin()->value, 20);
            map.erase(last);
            EXPECT_TRUE(map.empty());
        }

        TEST(SlotMapTest, CompactionReleasesOnlyTheErasedResource)
        {
            SlotMap<std::shared_ptr<int>> map;
            const auto erased_resource   = std::make_shared<int>(10);
            const auto retained_resource = std::make_shared<int>(20);
            const auto erased            = map.insert(erased_resource);
            const auto retained          = map.insert(retained_resource);

            map.erase(erased);

            EXPECT_EQ(erased_resource.use_count(), 1);
            EXPECT_EQ(retained_resource.use_count(), 2);
            ASSERT_TRUE(map.contains(retained));
            EXPECT_EQ(map.get(retained), retained_resource);
            map.erase(retained);
            EXPECT_EQ(retained_resource.use_count(), 1);
            EXPECT_TRUE(map.empty());
        }

        TEST(SlotMapTest, EraseDestroysTheStoredValueImmediately)
        {
            SlotMap<std::shared_ptr<int>> map;
            const auto source = std::make_shared<int>(42);
            const auto handle = map.insert(source);
            ASSERT_EQ(source.use_count(), 2);

            map.erase(handle);

            EXPECT_EQ(source.use_count(), 1);
            map.erase(handle);
            EXPECT_EQ(source.use_count(), 1);
        }

        TEST(SlotMapTest, DestructionReleasesAllRemainingValues)
        {
            const auto source = std::make_shared<int>(42);
            {
                SlotMap<std::shared_ptr<int>> map;
                const auto erased = map.insert(source);
                map.insert(source);
                map.insert(source);
                ASSERT_EQ(source.use_count(), 4);
                map.erase(erased);
                EXPECT_EQ(source.use_count(), 3);
            }
            EXPECT_EQ(source.use_count(), 1);
        }

        struct ThrowOnCopy {
            explicit ThrowOnCopy(int initial_value, bool should_fail = false)
                : value(initial_value)
                , fail(should_fail)
            {
            }

            ThrowOnCopy(const ThrowOnCopy& other)
                : value(other.value)
                , fail(other.fail)
            {
                if (fail) { throw std::runtime_error("copy failed"); }
            }

            ThrowOnCopy(ThrowOnCopy&&) noexcept = default;

            int value;
            bool fail;
        };

        TEST(SlotMapTest, FailedInsertionDoesNotExposeAnEmptySlot)
        {
            SlotMap<ThrowOnCopy> map;
            const ThrowOnCopy failing{42, true};

            EXPECT_THROW(map.insert(failing), std::runtime_error);

            EXPECT_TRUE(map.empty());
            EXPECT_EQ(map.size(), 0u);
            EXPECT_EQ(map.begin(), map.end());
            const SlotMap<ThrowOnCopy>::Handle vacant{0, 0};
            EXPECT_FALSE(map.contains(vacant));
            EXPECT_EQ(map.try_get(vacant), nullptr);
            const auto recovered = map.insert(ThrowOnCopy{99});
            ASSERT_TRUE(map.contains(recovered));
            EXPECT_EQ(map.get(recovered).value, 99);
        }

        TEST(SlotMapTest, FailedInsertionPreservesLiveValuesAndFreeSlot)
        {
            SlotMap<ThrowOnCopy> map;
            const auto live   = map.insert(ThrowOnCopy{10});
            const auto erased = map.insert(ThrowOnCopy{20});
            map.erase(erased);
            const ThrowOnCopy failing{30, true};

            EXPECT_THROW(map.insert(failing), std::runtime_error);

            ASSERT_EQ(map.size(), 1u);
            EXPECT_EQ(map.begin()->value, 10);
            ASSERT_TRUE(map.contains(live));
            EXPECT_EQ(map.get(live).value, 10);
            EXPECT_FALSE(map.contains(erased));
            const auto recovered = map.insert(ThrowOnCopy{40});
            EXPECT_EQ(recovered.index, erased.index);
            EXPECT_NE(recovered.generation, erased.generation);
            EXPECT_EQ(map.get(recovered).value, 40);
            EXPECT_EQ(map.get(live).value, 10);
            EXPECT_EQ(map.size(), 2u);
        }

        TEST(SlotMapTest, FailedCopyAssignmentPreservesDestinationAndItsFreeSlots)
        {
            SlotMap<ThrowOnCopy> source;
            source.insert(ThrowOnCopy{10});
            const auto failing       = source.insert(ThrowOnCopy{20});
            source.get(failing).fail = true;

            SlotMap<ThrowOnCopy> destination;
            const auto erased = destination.insert(ThrowOnCopy{100});
            const auto live   = destination.insert(ThrowOnCopy{200});
            destination.erase(erased);

            EXPECT_THROW(destination = source, std::runtime_error);

            ASSERT_EQ(destination.size(), 1u);
            EXPECT_EQ(destination.begin()->value, 200);
            ASSERT_TRUE(destination.contains(live));
            EXPECT_EQ(destination.get(live).value, 200);
            EXPECT_FALSE(destination.contains(erased));
            const auto reused = destination.insert(ThrowOnCopy{300});
            EXPECT_EQ(reused.index, erased.index);
            EXPECT_NE(reused.generation, erased.generation);
            EXPECT_EQ(destination.get(reused).value, 300);
            EXPECT_EQ(destination.get(live).value, 200);
            EXPECT_EQ(source.size(), 2u);
            EXPECT_EQ(source.get(failing).value, 20);
        }
    } // namespace
} // namespace orion
