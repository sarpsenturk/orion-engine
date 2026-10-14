#include "orion/lib/inline_function.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <functional>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace orion
{
    namespace
    {
        using IntFunction  = InlineFunction<64, int(int)>;
        using VoidFunction = InlineFunction<64, void()>;

        int AddOne(int value)
        {
            return value + 1;
        }

        struct LargeCallable {
            char padding[128]{};
            int operator()() const noexcept { return 7; }
        };

        struct ThrowingMoveCallable {
            ThrowingMoveCallable() = default;
            ThrowingMoveCallable(ThrowingMoveCallable&&) noexcept(false) {}
            int operator()() const noexcept { return 7; }
        };

        struct ThrowingCopyCallable {
            ThrowingCopyCallable() = default;
            ThrowingCopyCallable(const ThrowingCopyCallable&) noexcept(false) {}
            ThrowingCopyCallable(ThrowingCopyCallable&&) noexcept = default;
            int operator()() const noexcept { return 7; }
        };

        struct MoveOnlyCallable {
            explicit MoveOnlyCallable(int initial) noexcept
                : value(initial)
            {
            }
            MoveOnlyCallable(const MoveOnlyCallable&)            = delete;
            MoveOnlyCallable& operator=(const MoveOnlyCallable&) = delete;

            MoveOnlyCallable(MoveOnlyCallable&& other) noexcept
                : value(std::exchange(other.value, 0))
            {
            }

            int operator()(int increment) { return value += increment; }

            int value;
        };

        struct MutableCounter {
            explicit MutableCounter(int initial) noexcept
                : value(initial)
            {
            }
            int operator()(int increment) { return value += increment; }
            int value;
        };

        // Counts every target-object destruction separately from destruction of the
        // resource represented by the target. Keeping the counter pointers after a
        // move lets tests detect whether the moved-from target's destructor ran.
        struct LifetimeProbe {
            LifetimeProbe(int& destructors, int& resource_destructions) noexcept
                : destructors(&destructors)
                , resource_destructions(&resource_destructions)
                , owns_resource(true)
            {
            }

            LifetimeProbe(const LifetimeProbe&)            = delete;
            LifetimeProbe& operator=(const LifetimeProbe&) = delete;

            LifetimeProbe(LifetimeProbe&& other) noexcept
                : destructors(other.destructors)
                , resource_destructions(other.resource_destructions)
                , owns_resource(std::exchange(other.owns_resource, false))
            {
            }

            ~LifetimeProbe() noexcept
            {
                ++*destructors;
                if (owns_resource) {
                    ++*resource_destructions;
                }
            }

            void operator()() const noexcept {}

            int* destructors;
            int* resource_destructions;
            bool owns_resource;
        };

        using TraitsFunction = orion::InlineFunction<64, int()>;
        static_assert(!std::is_copy_constructible_v<TraitsFunction>);
        static_assert(!std::is_copy_assignable_v<TraitsFunction>);
        static_assert(std::is_nothrow_move_constructible_v<TraitsFunction>);
        static_assert(std::is_nothrow_move_assignable_v<TraitsFunction>);
        static_assert(!std::is_constructible_v<orion::InlineFunction<8, int()>, LargeCallable>);
        static_assert(!std::is_constructible_v<TraitsFunction, ThrowingMoveCallable>);
        static_assert(!std::is_constructible_v<TraitsFunction, ThrowingCopyCallable&>);
        static_assert(std::is_constructible_v<TraitsFunction, ThrowingCopyCallable&&>);

        TEST(InlineFunctionTest, DefaultConstructionIsEmpty)
        {
            IntFunction function;

            EXPECT_TRUE(function.empty());
            EXPECT_FALSE(static_cast<bool>(function));
        }

        TEST(InlineFunctionTest, EmptyFunctionThrowsBadFunctionCall)
        {
            IntFunction function;

            EXPECT_THROW(function(123), std::bad_function_call);
        }

        TEST(InlineFunctionTest, ResetOnEmptyFunctionIsSafeAndIdempotent)
        {
            IntFunction function;

            EXPECT_NO_THROW(function.reset());
            EXPECT_NO_THROW(function.reset());
            EXPECT_TRUE(function.empty());
        }

        TEST(InlineFunctionTest, InvokesLambdaAndReturnsValue)
        {
            IntFunction function{[](int value) { return value * 3; }};

            EXPECT_TRUE(function);
            EXPECT_EQ(function(14), 42);
        }

        TEST(InlineFunctionTest, AcceptsFunctionPointer)
        {
            IntFunction function{&AddOne};

            EXPECT_EQ(function(41), 42);
        }

        TEST(InlineFunctionTest, CopiesAnLvalueCallableIntoItsStorage)
        {
            MutableCounter original{10};
            IntFunction function{original};

            EXPECT_EQ(function(2), 12);
            EXPECT_EQ(function(3), 15);
            EXPECT_EQ(original.value, 10);
        }

        TEST(InlineFunctionTest, SupportsMutableCallableThroughConstWrapper)
        {
            orion::InlineFunction<64, int()> function{[value = 0]() mutable {
                return ++value;
            }};
            const auto& const_function = function;

            EXPECT_EQ(const_function(), 1);
            EXPECT_EQ(const_function(), 2);
        }

        TEST(InlineFunctionTest, SupportsVoidReturnAndReferenceArguments)
        {
            orion::InlineFunction<64, void(int&)> function{[](int& value) {
                ++value;
            }};
            int value = 41;

            function(value);

            EXPECT_EQ(value, 42);
        }

        TEST(InlineFunctionTest, SupportsReferenceReturn)
        {
            int value = 10;
            orion::InlineFunction<64, int&()> function{[&value]() -> int& {
                return value;
            }};

            function() = 42;

            EXPECT_EQ(value, 42);
        }

        TEST(InlineFunctionTest, PropagatesExceptionsThrownByCallable)
        {
            IntFunction function{[](int) -> int {
                throw std::runtime_error("callable failed");
            }};

            EXPECT_THROW(function(0), std::runtime_error);
        }

        TEST(InlineFunctionTest, SupportsMoveOnlyCallable)
        {
            orion::InlineFunction<64, int(int)> function{MoveOnlyCallable{40}};

            EXPECT_EQ(function(2), 42);
            EXPECT_EQ(function(1), 43);
        }

        TEST(InlineFunctionTest, MoveConstructionTransfersCallableAndEmptiesSource)
        {
            IntFunction source{[](int value) { return value + 10; }};

            IntFunction destination{std::move(source)};

            EXPECT_TRUE(source.empty());
            EXPECT_FALSE(destination.empty());
            EXPECT_THROW(source(1), std::bad_function_call);
            EXPECT_EQ(destination(32), 42);
        }

        TEST(InlineFunctionTest, MoveAssignmentReplacesExistingCallable)
        {
            IntFunction destination{[](int value) { return value + 100; }};
            IntFunction source{[](int value) { return value * 2; }};

            destination = std::move(source);

            EXPECT_TRUE(source.empty());
            EXPECT_EQ(destination(21), 42);
        }

        TEST(InlineFunctionTest, MoveAssignmentFromEmptyResetsDestination)
        {
            IntFunction destination{[](int value) { return value + 1; }};
            IntFunction empty_source;

            destination = std::move(empty_source);

            EXPECT_TRUE(destination.empty());
            EXPECT_TRUE(empty_source.empty());
            EXPECT_THROW(destination(41), std::bad_function_call);
        }

        TEST(InlineFunctionTest, SelfMoveAssignmentPreservesCallable)
        {
            IntFunction function{[](int value) { return value + 1; }};

            function = std::move(function);

            EXPECT_TRUE(function);
            EXPECT_EQ(function(41), 42);
        }

        TEST(InlineFunctionTest, MovedFromTargetIsDestroyedDuringMoveConstruction)
        {
            int object_destructors    = 0;
            int resource_destructions = 0;

            {
                VoidFunction source{LifetimeProbe{object_destructors, resource_destructions}};
                const int before_move = object_destructors; // Includes the temporary.

                {
                    VoidFunction destination{std::move(source)};

                    EXPECT_TRUE(source.empty());
                    EXPECT_EQ(object_destructors, before_move + 1)
                        << "The moved-from target object must be destroyed by the move thunk.";
                    EXPECT_EQ(resource_destructions, 0)
                        << "Moving must transfer resource ownership, not destroy the resource.";
                    EXPECT_NO_THROW(destination());
                }

                EXPECT_EQ(resource_destructions, 1);
                EXPECT_EQ(object_destructors, before_move + 2);
            }

            EXPECT_EQ(resource_destructions, 1);
        }

        TEST(InlineFunctionTest, MoveAssignmentDestroysOldTargetAndMovedFromSource)
        {
            int object_destructors    = 0;
            int resource_destructions = 0;

            {
                VoidFunction destination{LifetimeProbe{object_destructors, resource_destructions}};
                VoidFunction source{LifetimeProbe{object_destructors, resource_destructions}};
                const int before_assignment = object_destructors; // Includes both temporaries.

                destination = std::move(source);

                EXPECT_TRUE(source.empty());
                EXPECT_EQ(object_destructors, before_assignment + 2)
                    << "Assignment must destroy the destination's old target and the moved-from source target.";
                EXPECT_EQ(resource_destructions, 1)
                    << "Only the resource formerly owned by the destination should be destroyed.";
                EXPECT_NO_THROW(destination());
            }

            EXPECT_EQ(resource_destructions, 2);
        }

        TEST(InlineFunctionTest, ResetDestroysTargetExactlyOnce)
        {
            int object_destructors    = 0;
            int resource_destructions = 0;

            {
                VoidFunction function{LifetimeProbe{object_destructors, resource_destructions}};
                const int after_construction = object_destructors; // The temporary has died.

                function.reset();
                EXPECT_TRUE(function.empty());
                EXPECT_EQ(object_destructors, after_construction + 1);
                EXPECT_EQ(resource_destructions, 1);

                function.reset();
                EXPECT_EQ(object_destructors, after_construction + 1);
                EXPECT_EQ(resource_destructions, 1);
            }

            EXPECT_EQ(resource_destructions, 1);
        }

    } // namespace
} // namespace orion
