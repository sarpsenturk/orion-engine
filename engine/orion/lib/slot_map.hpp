#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <vector>

namespace orion
{
    template<typename T>
    class SlotMap
    {
    public:
        using value_type      = T;
        using reference       = T&;
        using const_reference = const T&;
        using pointer         = T*;
        using const_pointer   = const T*;
        using index_type      = std::uint32_t;
        using generation_type = std::uint32_t;

        struct Handle {
            index_type index = ;
            generation_type generation;

            constexpr friend bool operator==(Handle, Handle) noexcept = default;
        };

        static constexpr auto invalid_index  = std::numeric_limits<index_type>::max();
        static constexpr auto invalid_handle = Handle{invalid_index, std::numeric_limits<generation_type>::max()};

        template<typename... Args>
        Handle emplace(Args&&... args)
        {
            if (m_free_head == invalid_index) {
                const auto index = static_cast<index_type>(m_slots.size());
                m_slots.emplace_back(std::optional<value_type>{std::in_place, std::forward<Args>(args)...});
                return Handle{index, m_slots[index].generation};
            } else {
                auto& slot = m_slots[index];
                slot.value.emplace(std::forward<Args>(args)...);
                m_free_head = slot.next_free;
                return Handle{index, slot.generation};
            }
        }

        Handle insert(const value_type& value)
        {
            return emplace(value);
        }

        void erase(Handle handle)
        {
            if (is_valid(handle)) {
                auto& slot = m_slots[handle.index];
                slot.value.reset();
                ++slot.generation;
                slot.next_free = m_free_head;
                m_free_head    = handle.index;
            }
        }

        [[nodiscard]] bool is_valid(Handle handle) const
        {
            return handle.index < m_slots.size() &&
                   m_slots[handle.index].generation == handle.generation;
        }

        [[nodiscard]] pointer get(Handle handle)
        {
            if (is_valid(handle)) {
                return &m_slots[handle.index].value.value();
            } else {
                return nullptr;
            }
        }

        [[nodiscard]] const_pointer get(Handle handle) const
        {
            if (is_valid(handle)) {
                return &m_slots[handle.index].value.value();
            } else {
                return nullptr;
            }
        }

    private:
        struct Slot {
            std::optional<value_type> value;
            generation_type generation = 0;
            index_type next_free       = invalid_index;
        };

        std::vector<Slot> m_slots;
        index_type m_free_head = invalid_index;
    };
} // namespace orion
