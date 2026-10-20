#pragma once

// Based on Sebastian Aaltonen's OffsetAllocator (MIT license):
// https://github.com/sebbbi/OffsetAllocator

#include <array>
#include <cstdint>
#include <limits>
#include <vector>

namespace orion
{
    // Manages offsets only; all metadata stays on the CPU. Not thread safe.
    class OffsetAllocator
    {
    public:
        static constexpr std::uint32_t invalid_index = std::numeric_limits<std::uint32_t>::max();

        struct Allocation {
            std::uint32_t offset   = invalid_index;
            std::uint32_t size     = 0;
            std::uint32_t metadata = invalid_index;

            explicit operator bool() const { return metadata != invalid_index; }
        };

        // max_nodes includes both allocated ranges and free ranges.
        explicit OffsetAllocator(std::uint32_t size, std::uint32_t max_nodes = 65'536);
        OffsetAllocator(const OffsetAllocator&)                = delete;
        OffsetAllocator& operator=(const OffsetAllocator&)     = delete;
        OffsetAllocator(OffsetAllocator&&) noexcept            = default;
        OffsetAllocator& operator=(OffsetAllocator&&) noexcept = default;
        ~OffsetAllocator()                                     = default;

        // Invalidates all allocations without allocating new metadata storage.
        void reset();

        // Alignment must be a nonzero power of two, relative to offset zero.
        // Returns an invalid allocation for zero size or exhausted space/metadata.
        // Bounded bin lookup can skip fitting blocks in a partially fitting bin.
        // Leading alignment padding is held until deallocate().
        [[nodiscard]] Allocation allocate(std::uint32_t size, std::uint32_t alignment = 1);
        // Requires a live allocation from this allocator; handles must not be reused.
        void deallocate(Allocation allocation);

        [[nodiscard]] std::uint32_t capacity() const { return m_capacity; }
        [[nodiscard]] std::uint32_t free_size() const { return m_free_size; }

    private:
        static constexpr std::uint32_t second_level_count = 8;
        static constexpr std::uint32_t bin_count          = 32 * second_level_count;

        struct Node {
            std::uint32_t offset        = 0;
            std::uint32_t size          = 0;
            std::uint32_t previous      = invalid_index;
            std::uint32_t next          = invalid_index;
            std::uint32_t free_previous = invalid_index;
            std::uint32_t free_next     = invalid_index;
            bool free                   = false;
        };

        static std::uint32_t bin_index(std::uint32_t size);
        std::uint32_t find_node(std::uint32_t size) const;
        void insert_free(std::uint32_t index);
        void remove_free(std::uint32_t index);
        std::uint32_t acquire_node();
        void release_node(std::uint32_t index);

        std::uint32_t m_capacity;
        std::uint32_t m_free_size   = 0;
        std::uint32_t m_unused      = invalid_index;
        std::uint32_t m_first_level = 0;
        std::array<std::uint32_t, 32> m_second_level{};
        std::array<std::uint32_t, bin_count> m_bins{};
        std::vector<Node> m_nodes;
    };
} // namespace orion
