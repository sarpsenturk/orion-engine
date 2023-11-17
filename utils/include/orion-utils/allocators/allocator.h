#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>

namespace orion
{
    struct Allocation {
        void* ptr;
        std::size_t size;
    };

    class Allocator
    {
    public:
        constexpr Allocator() = default;
        constexpr virtual ~Allocator() = default;

        [[nodiscard]] constexpr Allocation allocate(std::size_t size, std::size_t alignment)
        {
            return do_allocate(size, alignment);
        }
        constexpr void deallocate(const Allocation& allocation)
        {
            return do_deallocate(allocation);
        }

        // Total bytes available in allocator
        [[nodiscard]] constexpr virtual std::size_t max_available() const = 0;
        // Currently available bytes
        [[nodiscard]] constexpr virtual std::size_t available() const = 0;
        // Currently used bytes
        [[nodiscard]] constexpr virtual std::size_t used() const = 0;

    protected:
        Allocator(const Allocator&) = default;
        Allocator(Allocator&&) noexcept = default;
        Allocator& operator=(const Allocator&) = default;
        Allocator& operator=(Allocator&&) noexcept = default;

    private:
        constexpr virtual Allocation do_allocate(std::size_t size, std::size_t alignment) = 0;
        constexpr virtual void do_deallocate(const Allocation& allocation) = 0;
    };

    constexpr std::uintptr_t to_uintptr_t(const void* ptr) noexcept
    {
        return std::bit_cast<std::uintptr_t>(ptr);
    }

    constexpr std::size_t align_forward_required(const void* ptr, std::size_t alignment)
    {
        const auto uintptr = to_uintptr_t(ptr);
        const auto aligned = (uintptr - 1u + alignment) & -alignment;
        return aligned - uintptr;
    }
} // namespace orion
