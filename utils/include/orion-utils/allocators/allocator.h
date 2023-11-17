#pragma once

#include <bit>
#include <cstddef>
#include <cstdint>
#include <new>

namespace orion
{
    struct Allocation {
        void* ptr;
        std::size_t size;
    };

    class AllocatorBase
    {
    public:
        constexpr AllocatorBase() = default;
        constexpr virtual ~AllocatorBase() = default;

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
        AllocatorBase(const AllocatorBase&) = default;
        AllocatorBase(AllocatorBase&&) noexcept = default;
        AllocatorBase& operator=(const AllocatorBase&) = default;
        AllocatorBase& operator=(AllocatorBase&&) noexcept = default;

    private:
        constexpr virtual Allocation do_allocate(std::size_t size, std::size_t alignment) = 0;
        constexpr virtual void do_deallocate(const Allocation& allocation) = 0;
    };

    class DefaultAllocator : public AllocatorBase
    {
    public:
        [[nodiscard]] constexpr std::size_t max_available() const override
        {
            return SIZE_MAX;
        }
        [[nodiscard]] constexpr std::size_t available() const override
        {
            return SIZE_MAX;
        }
        [[nodiscard]] constexpr std::size_t used() const override
        {
            return bytes_allocated_;
        }

    private:
        Allocation do_allocate(std::size_t size, std::size_t alignment) override
        {
            void* ptr = ::operator new(size, static_cast<std::align_val_t>(alignment));
            bytes_allocated_ += size;
            return {ptr, size};
        }

        void do_deallocate(const Allocation& allocation) override
        {
            ::operator delete(allocation.ptr, allocation.size);
            bytes_allocated_ -= allocation.size;
        }

        std::size_t bytes_allocated_ = 0ull;
    };

    template<typename A>
    concept Allocator = requires(A allocator, std::size_t size, std::size_t alignment, const Allocation& allocation) {
        allocator.allocate(size, alignment);
        allocator.deallocate(allocation);
    };

    template<typename T, Allocator A>
    class STLAllocatorAdapter
    {
    public:
        using value_type = T;
        using allocator_type = A;

        constexpr explicit STLAllocatorAdapter(std::reference_wrapper<allocator_type> allocator)
            : allocator_(allocator)
        {
        }

        constexpr value_type* allocate(std::size_t n)
        {
            return static_cast<value_type*>(allocator().allocate(sizeof(value_type) * n, alignof(value_type)));
        }

        constexpr void deallocate(value_type* ptr, std::size_t n)
        {
            allocator().deallocate(Allocation{.ptr = ptr, .size = sizeof(value_type) * n});
        }

        constexpr allocator_type& allocator() { return allocator_.get(); }

        constexpr bool operator==(const STLAllocatorAdapter& other) const noexcept
        {
            return &(allocator()) == &(other.allocator());
        }

        constexpr bool operator!=(const STLAllocatorAdapter& other) const noexcept { return !(*this == other); }

    private:
        std::reference_wrapper<allocator_type> allocator_;
    };

    template<typename T>
    using STLDefaultAllocator = STLAllocatorAdapter<T, DefaultAllocator>;

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
