#pragma once

#include <cstddef>
#include <limits>

namespace orion::gpu
{
    class BumpAllocator
    {
    public:
        constexpr BumpAllocator() = default;
        constexpr explicit BumpAllocator(std::size_t size)
            : m_size(size)
        {
        }

        constexpr std::size_t allocate(std::size_t size, std::size_t alignment = 1z)
        {
            const auto aligned_offset = (m_offset + alignment - 1z) & ~(alignment - 1z);
            if (aligned_offset + size > m_size) {
                return std::numeric_limits<std::size_t>::max();
            }
            m_offset = aligned_offset + size;
            return aligned_offset;
        }

        constexpr void reset()
        {
            m_offset = 0z;
        }

    private:
        std::size_t m_size   = 0z;
        std::size_t m_offset = 0z;
    };
} // namespace orion::gpu
