#pragma once

#include <cstddef>

namespace orion::gpu
{
    // Manages ranges only; backing storage and synchronization belong to the caller.
    // Operations on the same allocator require external synchronization.
    class RangeAllocator
    {
    public:
        enum class Algorithm {
            Default = 0, // Reuses free ranges in arbitrary allocation/free order.
            Linear,      // Supports bulk reset, LIFO stacks, and FIFO rings; does not reuse middle holes.
        };

        struct Allocation {
            void* handle       = nullptr;
            std::size_t offset = 0z;
            std::size_t size   = 0z;

            explicit operator bool() const noexcept { return handle != nullptr; }
        };

        // Capacity must be nonzero. All sizes, offsets, and alignments use the same units.
        explicit RangeAllocator(std::size_t size, Algorithm algorithm = Algorithm::Default);
        RangeAllocator(const RangeAllocator&) = delete;
        RangeAllocator& operator=(const RangeAllocator&) = delete;
        RangeAllocator(RangeAllocator&& other) noexcept;
        RangeAllocator& operator=(RangeAllocator&& other) noexcept;
        ~RangeAllocator();

        // Size must be nonzero and alignment a nonzero power of two.
        // Returns {handle = nullptr} if allocation fails; offset zero is valid when handle is non-null.
        [[nodiscard]] Allocation allocate(std::size_t size, std::size_t alignment = 1z);
        void free(const Allocation& allocation);
        // Frees all ranges and invalidates every outstanding allocation handle.
        void reset();

        // Sum of live allocation sizes, excluding alignment gaps, in allocator units.
        [[nodiscard]] std::size_t used_bytes() const noexcept;

    private:
        void* m_block = nullptr;
    };
} // namespace orion::gpu
