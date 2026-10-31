#include "orion/utils/inplace_function.hpp"

#include <gtest/gtest.h>

#include <stdexcept>

namespace orion
{
    namespace
    {
        using Function = InplaceFunction<32, int(int)>;
        struct Large { char bytes[64]; int operator()(int) { return 0; } };
        struct alignas(64) Aligned { int operator()(int) { return 0; } };
        struct ThrowingMove {
            ThrowingMove(const ThrowingMove&) = default;
            ThrowingMove(ThrowingMove&&) noexcept(false) {}
            int operator()(int n) { return n; }
        };
        static_assert(!std::is_copy_constructible_v<Function>);
        static_assert(!std::is_copy_assignable_v<Function>);
        static_assert(std::is_nothrow_move_constructible_v<Function>);
        static_assert(std::is_nothrow_move_assignable_v<Function>);
        static_assert(!std::is_constructible_v<Function, ThrowingMove>);
        static_assert(!std::is_constructible_v<Function, ThrowingMove&>);
        static_assert(!std::is_constructible_v<Function, const ThrowingMove&>);
        static_assert(!std::is_constructible_v<Function, Large>);
        static_assert(!std::is_constructible_v<InplaceFunction<128, int(int)>, Aligned>);
        static_assert(!std::is_constructible_v<Function, int>);

        TEST(InplaceFunctionTest, EmptyAndNullTargets)
        {
            Function empty;
            EXPECT_FALSE(empty);
            EXPECT_THROW(empty(1), std::bad_function_call);
            Function null(nullptr);
            EXPECT_FALSE(null);
            int (*pointer)(int) = nullptr;
            Function function(pointer);
            EXPECT_FALSE(function);
            InplaceFunction<32, int(Large&, int)> member(static_cast<int (Large::*)(int)>(nullptr));
            EXPECT_FALSE(member);
        }

        TEST(InplaceFunctionTest, MutableAndMoveOnlyTargets)
        {
            const Function mutable_target([count = 0](int value) mutable { return count += value; });
            EXPECT_EQ(mutable_target(2), 2);
            EXPECT_EQ(mutable_target(3), 5);
            Function original([value = std::make_unique<int>(7)](int n) { return *value + n; });
            Function moved(std::move(original));
            EXPECT_FALSE(original);
            EXPECT_EQ(moved(2), 9);
            Function assigned;
            assigned = std::move(moved);
            EXPECT_FALSE(moved);
            EXPECT_EQ(assigned(3), 10);
            auto& alias = assigned;
            assigned = std::move(alias);
            EXPECT_EQ(assigned(1), 8);
            assigned = [](int n) { return n * 2; };
            EXPECT_EQ(assigned(3), 6);
        }

        TEST(InplaceFunctionTest, ForwardsArgumentsAndReferences)
        {
            InplaceFunction<32, int(std::unique_ptr<int>)> consume([](std::unique_ptr<int> p) { return *p; });
            EXPECT_EQ(consume(std::make_unique<int>(42)), 42);
            InplaceFunction<32, int&(int&)> reference([](int& n) -> int& { return n; });
            int value = 4;
            reference(value) = 9;
            EXPECT_EQ(value, 9);
            InplaceFunction<32, void(int&)> discard([](int& n) { return ++n; });
            discard(value);
            EXPECT_EQ(value, 10);
            struct Object { int value; int add(int n) { return value + n; } };
            Object object{3};
            InplaceFunction<32, int(Object&, int)> member(&Object::add);
            EXPECT_EQ(member(object, 4), 7);
        }

        TEST(InplaceFunctionTest, ReleasesOwnershipOnReplacementResetAndDestruction)
        {
            auto owner = std::make_shared<int>(1);
            {
                Function first([owner](int n) { return n; });
                Function second([owner](int n) { return n; });
                EXPECT_EQ(owner.use_count(), 3);
                second = std::move(first);
                EXPECT_EQ(owner.use_count(), 2);
                second.reset();
                second.reset();
                EXPECT_EQ(owner.use_count(), 1);
                second = [owner](int n) { return n; };
                second = nullptr;
                EXPECT_EQ(owner.use_count(), 1);
                second = [owner](int n) { return n; };
            }
            EXPECT_EQ(owner.use_count(), 1);
        }

        TEST(InplaceFunctionTest, ExactCapacityAndLvalueCopy)
        {
            struct Target { char bytes[32]{}; int operator()(int n) { return n + bytes[0]; } };
            Target target;
            target.bytes[0] = 5;
            Function function(target);
            target.bytes[0] = 9;
            EXPECT_EQ(function(1), 6);
        }

        TEST(InplaceFunctionTest, ThrowingCopyPreservesDestination)
        {
            struct Target {
                bool* fail;
                Target(bool& flag) : fail(&flag) {}
                Target(const Target& other) : fail(other.fail) { if (*fail) throw std::runtime_error("copy"); }
                Target(Target&&) noexcept = default;
                int operator()(int n) { return n; }
            };
            static_assert(std::is_nothrow_constructible_v<Function, Target>);
            static_assert(!std::is_nothrow_constructible_v<Function, Target&>);
            bool fail = true;
            Target target{fail};
            Function destination([](int n) { return n * 2; });
            EXPECT_THROW(destination = target, std::runtime_error);
            EXPECT_EQ(destination(3), 6);
            Function source(std::move(target));
            destination = std::move(source);
            EXPECT_EQ(destination(4), 4);
            EXPECT_FALSE(source);
        }
    }
} // namespace orion
