#include "offset_allocator.hpp"

#include "orion/core/assert.hpp"

#include <bit>

namespace orion
{
    OffsetAllocator::OffsetAllocator(std::uint32_t size, std::uint32_t max_nodes)
        : m_capacity(size)
        , m_nodes(max_nodes)
    {
        reset();
    }

    void OffsetAllocator::reset()
    {
        m_free_size   = m_capacity;
        m_first_level = 0;
        m_second_level.fill(0);
        m_bins.fill(invalid_index);
        for (std::uint32_t i = 0; i < m_nodes.size(); ++i) {
            m_nodes[i] = Node{.free_next = i + 1 < m_nodes.size() ? i + 1 : invalid_index};
        }
        m_unused = m_nodes.empty() ? invalid_index : 0;
        if (m_capacity != 0 && m_unused != invalid_index) {
            const auto index = acquire_node();
            m_nodes[index]   = Node{.size = m_capacity};
            insert_free(index);
        }
    }

    std::uint32_t OffsetAllocator::bin_index(std::uint32_t size)
    {
        const auto first  = static_cast<std::uint32_t>(std::bit_width(size) - 1);
        const auto shift  = first > 3 ? first - 3 : 0;
        const auto second = (size - (std::uint32_t{1} << first)) >> shift;
        return first * second_level_count + second;
    }

    std::uint32_t OffsetAllocator::find_node(std::uint32_t size) const
    {
        const auto bin  = bin_index(size);
        const auto head = m_bins[bin];
        if (head != invalid_index && m_nodes[head].size >= size) return head;

        auto first     = bin / second_level_count;
        auto available = m_second_level[first] & (~std::uint32_t{0} << (bin % second_level_count + 1));
        if (available == 0) {
            const auto higher = m_first_level & static_cast<std::uint32_t>(~std::uint64_t{0} << (first + 1));
            if (higher == 0) return invalid_index;
            first     = static_cast<std::uint32_t>(std::countr_zero(higher));
            available = m_second_level[first];
        }
        return m_bins[first * second_level_count + std::countr_zero(available)];
    }

    void OffsetAllocator::insert_free(std::uint32_t index)
    {
        auto& node         = m_nodes[index];
        const auto bin     = bin_index(node.size);
        node.free          = true;
        node.free_previous = invalid_index;
        node.free_next     = m_bins[bin];
        if (node.free_next != invalid_index) m_nodes[node.free_next].free_previous = index;
        m_bins[bin] = index;
        m_first_level |= std::uint32_t{1} << (bin / second_level_count);
        m_second_level[bin / second_level_count] |= std::uint32_t{1} << (bin % second_level_count);
    }

    void OffsetAllocator::remove_free(std::uint32_t index)
    {
        auto& node     = m_nodes[index];
        const auto bin = bin_index(node.size);
        if (node.free_previous != invalid_index)
            m_nodes[node.free_previous].free_next = node.free_next;
        else
            m_bins[bin] = node.free_next;
        if (node.free_next != invalid_index) m_nodes[node.free_next].free_previous = node.free_previous;
        if (m_bins[bin] == invalid_index) {
            m_second_level[bin / second_level_count] &= ~(std::uint32_t{1} << (bin % second_level_count));
            if (m_second_level[bin / second_level_count] == 0) m_first_level &= ~(std::uint32_t{1} << (bin / second_level_count));
        }
        node.free = false;
    }

    std::uint32_t OffsetAllocator::acquire_node()
    {
        const auto index = m_unused;
        m_unused         = m_nodes[index].free_next;
        return index;
    }

    void OffsetAllocator::release_node(std::uint32_t index)
    {
        m_nodes[index].free_next = m_unused;
        m_unused                 = index;
    }

    OffsetAllocator::Allocation OffsetAllocator::allocate(std::uint32_t size, std::uint32_t alignment)
    {
        ORION_ASSERT(std::has_single_bit(alignment));
        if (size == 0 || size > m_free_size) return {};
        auto index = find_node(size);
        if (index == invalid_index) return {};
        const auto padding_for = [alignment](std::uint32_t offset) { return (0u - offset) & (alignment - 1); };
        auto padding           = padding_for(m_nodes[index].offset);
        if (padding > m_nodes[index].size - size) {
            const auto required = static_cast<std::uint64_t>(size) + alignment - 1;
            if (required > m_capacity) return {};
            index = find_node(static_cast<std::uint32_t>(required));
            if (index == invalid_index) return {};
            padding = padding_for(m_nodes[index].offset);
        }

        auto& node           = m_nodes[index];
        const auto consumed  = size + padding;
        const auto remainder = node.size - consumed;
        if (remainder != 0 && m_unused == invalid_index) return {};
        remove_free(index);
        if (remainder != 0) {
            const auto tail = acquire_node();
            m_nodes[tail]   = Node{.offset = node.offset + consumed, .size = remainder, .previous = index, .next = node.next};
            if (node.next != invalid_index) m_nodes[node.next].previous = tail;
            node.next = tail;
            insert_free(tail);
        }
        node.size = consumed;
        m_free_size -= consumed;
        return Allocation{.offset = node.offset + padding, .size = size, .metadata = index};
    }

    void OffsetAllocator::deallocate(Allocation allocation)
    {
        ORION_ASSERT(allocation.metadata < m_nodes.size());
        auto& node = m_nodes[allocation.metadata];
        ORION_ASSERT(!node.free);
        m_free_size += node.size;
        if (node.previous != invalid_index && m_nodes[node.previous].free) {
            const auto index     = node.previous;
            const auto& previous = m_nodes[index];
            remove_free(index);
            node.offset = previous.offset;
            node.size += previous.size;
            node.previous = previous.previous;
            release_node(index);
        }
        if (node.next != invalid_index && m_nodes[node.next].free) {
            const auto index = node.next;
            const auto& next = m_nodes[index];
            remove_free(index);
            node.size += next.size;
            node.next = next.next;
            release_node(index);
        }
        if (node.previous != invalid_index) m_nodes[node.previous].next = allocation.metadata;
        if (node.next != invalid_index) m_nodes[node.next].previous = allocation.metadata;
        insert_free(allocation.metadata);
    }
} // namespace orion
