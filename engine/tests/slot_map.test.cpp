#include "orion/utils/slot_map.hpp"

#include <gtest/gtest.h>

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
            const auto live = map.insert(42);
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
            const auto first  = map.insert(10);
            const auto live   = map.insert(20);
            const auto last   = map.insert(30);
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
            constexpr int count = 1024;

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
            {}

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
                : value(initial_value),
                  fail(should_fail)
            {}

            ThrowOnCopy(const ThrowOnCopy& other)
                : value(other.value),
                  fail(other.fail)
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

            ASSERT_TRUE(map.contains(live));
            EXPECT_EQ(map.get(live).value, 10);
            EXPECT_FALSE(map.contains(erased));
            const auto recovered = map.insert(ThrowOnCopy{40});
            EXPECT_EQ(recovered.index, erased.index);
            EXPECT_NE(recovered.generation, erased.generation);
            EXPECT_EQ(map.get(recovered).value, 40);
            EXPECT_EQ(map.get(live).value, 10);
        }
    }
} // namespace orion
