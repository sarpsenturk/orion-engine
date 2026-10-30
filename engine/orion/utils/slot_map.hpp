#pragma once

#include "orion/core/assert.hpp"

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace orion
{
    template<typename T>
    class SlotMap
    {
        // Erasing fills a gap by relocating the last value. Relocation must not
        // throw after the erased value has been destroyed.
        static_assert(std::is_nothrow_move_constructible_v<T> || std::is_nothrow_copy_constructible_v<T>,
                      "SlotMap values must support nonthrowing move or copy construction");
        static_assert(std::is_nothrow_destructible_v<T>, "SlotMap values must have nonthrowing destructors");

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

        SlotMap()               = default;
        SlotMap(const SlotMap&) = default;
        SlotMap& operator=(const SlotMap& other)
        {
            if (this != &other) {
                SlotMap copy(other);
                swap(copy);
            }
            return *this;
        }

        SlotMap(SlotMap&& other) noexcept { swap(other); }
        SlotMap& operator=(SlotMap&& other) noexcept
        {
            if (this != &other) {
                SlotMap moved(std::move(other));
                swap(moved);
            }
            return *this;
        }

        void swap(SlotMap& other) noexcept
        {
            m_slots.swap(other.m_slots);
            m_values.swap(other.m_values);
            m_value_slots.swap(other.m_value_slots);
            std::swap(m_free_head, other.m_free_head);
        }

        Handle insert(const T& value)
        {
            auto index = m_free_head;
            if (index == invalid_index) {
                if (m_slots.size() == invalid_index) {
                    throw std::length_error("SlotMap has exhausted its handle indices");
                }
                index = static_cast<index_type>(m_slots.size());
                m_slots.emplace_back();
                m_free_head = index;
            }

            // Keep the handle slot vacant until both dense arrays are populated.
            m_value_slots.push_back(index);
            try {
                m_values.push_back(value);
            } catch (...) {
                m_value_slots.pop_back();
                throw;
            }

            auto& slot           = m_slots[index];
            slot.value_index     = static_cast<index_type>(m_values.size() - 1);
            m_free_head          = slot.next_free_index;
            slot.next_free_index = invalid_index;
            return Handle{index, slot.generation};
        }

        void erase(Handle handle)
        {
            if (!contains(handle)) { return; }
            auto& slot             = m_slots[handle.index];
            const auto value_index = slot.value_index;
            const auto last_index  = static_cast<index_type>(m_values.size() - 1);
            if (value_index != last_index) {
                // Construct in place instead of assigning, so T need not be
                // assignable (for example, records with const members).
                auto* destination = std::addressof(m_values[value_index]);
                std::destroy_at(destination);
                std::construct_at(destination, std::move_if_noexcept(m_values.back()));
                const auto moved_slot           = m_value_slots.back();
                m_value_slots[value_index]      = moved_slot;
                m_slots[moved_slot].value_index = value_index;
            }
            m_values.pop_back();
            m_value_slots.pop_back();
            slot.value_index = invalid_index;
            slot.generation += 1;
            slot.next_free_index = m_free_head;
            m_free_head          = handle.index;
        }

        [[nodiscard]] bool contains(Handle handle) const
        {
            return handle.index < m_slots.size() &&
                   m_slots[handle.index].generation == handle.generation &&
                   m_slots[handle.index].value_index != invalid_index;
        }

        template<typename Self>
        [[nodiscard]] auto&& get(this Self&& self, Handle handle)
        {
            ORION_ASSERT(self.contains(handle));
            return std::forward_like<Self>(self.m_values[self.m_slots[handle.index].value_index]);
        }

        template<typename Self>
        [[nodiscard]] auto* try_get(this Self&& self, Handle handle)
        {
            return self.contains(handle)
                       ? std::addressof(self.m_values[self.m_slots[handle.index].value_index])
                       : nullptr;
        }

        [[nodiscard]] std::size_t size() const noexcept { return m_values.size(); }
        [[nodiscard]] bool empty() const noexcept { return m_values.empty(); }

        // Iterates only live values, in unspecified order. Handles survive
        // insertion and unrelated erasure. Insertion can invalidate all value
        // references/iterators; erasure invalidates those to the erased value and
        // the last value, which moves into its place. Do not mutate the map's
        // structure during a range-for loop; modifying individual values is fine.
        [[nodiscard]] auto begin() noexcept { return m_values.begin(); }
        [[nodiscard]] auto end() noexcept { return m_values.end(); }
        [[nodiscard]] auto begin() const noexcept { return m_values.begin(); }
        [[nodiscard]] auto end() const noexcept { return m_values.end(); }
        [[nodiscard]] auto cbegin() const noexcept { return m_values.cbegin(); }
        [[nodiscard]] auto cend() const noexcept { return m_values.cend(); }

    private:
        struct Slot {
            index_type value_index     = invalid_index;
            index_type next_free_index = invalid_index;
            generation_type generation = 0;
        };

        std::vector<Slot> m_slots;
        std::vector<T> m_values;
        // Dense value index -> stable handle slot, for repairing lookup on erase.
        std::vector<index_type> m_value_slots;
        index_type m_free_head = invalid_index;
    };
} // namespace orion
