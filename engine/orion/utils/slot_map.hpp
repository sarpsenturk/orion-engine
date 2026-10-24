#pragma once

#include "orion/core/assert.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace orion
{
    template<typename T>
    class SlotMap
    {
    public:
        using index_type      = std::uint32_t;
        using generation_type = std::uint32_t;

        static constexpr auto invalid_index = std::numeric_limits<index_type>::max();

        struct Handle {
            index_type index           = invalid_index;
            generation_type generation = 0;

            static constexpr Handle from_uint64(std::uint64_t value) noexcept
            {
                return Handle{
                    .index      = static_cast<index_type>(value & 0xFFFFFFFF),
                    .generation = static_cast<generation_type>(value >> 32),
                };
            }
            constexpr std::uint64_t to_uint64() const noexcept
            {
                return (static_cast<std::uint64_t>(generation) << 32) | index;
            }

            constexpr bool operator==(const Handle&) const noexcept = default;
        };

        Handle insert(const T& value)
        {
            auto index = m_free_head;
            if (index == invalid_index) {
                index = static_cast<index_type>(m_slots.size());
                m_slots.emplace_back();
                m_free_head = index;
            }
            auto& slot = m_slots[index];
            // Keep the slot on the free list until construction succeeds.
            slot.value.emplace(value);
            m_free_head          = slot.next_free_index;
            slot.next_free_index = invalid_index;
            return Handle{index, slot.generation};
        }

        void erase(Handle handle)
        {
            if (!contains(handle)) { return; }
            auto& slot = m_slots[handle.index];
            slot.value.reset();
            slot.generation += 1;
            slot.next_free_index = m_free_head;
            m_free_head          = handle.index;
        }

        [[nodiscard]] bool contains(Handle handle) const
        {
            return handle.index < m_slots.size() &&
                   m_slots[handle.index].generation == handle.generation &&
                   m_slots[handle.index].value.has_value();
        }

        template<typename Self>
        [[nodiscard]] auto&& get(this Self&& self, Handle handle)
        {
            ORION_ASSERT(self.contains(handle));
            return std::forward<Self>(self).m_slots[handle.index].value.value();
        }

        template<typename Self>
        [[nodiscard]] auto* try_get(this Self&& self, Handle handle)
        {
            return self.contains(handle)
                       ? std::addressof(self.m_slots[handle.index].value.value())
                       : nullptr;
        }

    private:
        struct Slot {
            std::optional<T> value;
            index_type next_free_index = invalid_index;
            generation_type generation = 0;
        };

        std::vector<Slot> m_slots;
        index_type m_free_head = invalid_index;
    };
} // namespace orion
