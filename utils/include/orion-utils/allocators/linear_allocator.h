#pragma once

#include "allocator.h"

#include <cstddef>
#include <stdexcept>
#include <vector>

namespace orion
{
    class LinearAllocator : public AllocatorBase
    {
    public:
        constexpr explicit LinearAllocator(std::size_t max_size)
            : memory_(max_size)
            , current_(memory_.data())
        {
        }

        [[nodiscard]] constexpr std::size_t max_available() const override
        {
            return memory_.size();
        }
        [[nodiscard]] constexpr std::size_t available() const override
        {
            return (memory_.data() + memory_.size()) - current_;
        }
        [[nodiscard]] constexpr std::size_t used() const override
        {
            return memory_.data() - current_;
        }

        [[nodiscard]] constexpr bool is_full() const noexcept { return available() == 0; }
        [[nodiscard]] constexpr bool is_empty() const noexcept { return available() == max_available(); }

        constexpr void reset() noexcept
        {
            current_ = memory_.data();
        }

    private:
        constexpr Allocation do_allocate(std::size_t size, std::size_t alignment) final
        {
            // Calculate required bytes to reach alignment
            const auto alignment_offset = align_forward_required(current_, alignment);

            // Out of memory if size + offset is more than available space
            if (size + alignment_offset > available()) {
                throw std::bad_alloc();
            }

            // Advance pointer by alignment offset
            void* ptr = (current_ += alignment_offset);

            // Advance pointer by size
            current_ += size;

            return {ptr, size};
        }

        void do_deallocate(const Allocation& /*allocation*/) final
        {
            throw std::logic_error("cannot deallocate individual allocations in a LinearAllocator");
        }

        std::vector<std::byte> memory_;
        std::byte* current_;
    };

    template<typename T>
    using STLLinearAllocator = STLAllocatorAdapter<T, LinearAllocator>;
} // namespace orion
