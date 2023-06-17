#pragma once

#include <array>       // std::array
#include <cstddef>     // std::byte
#include <iterator>    // std::forward_iterator, std::iterator_traits
#include <memory>      // std::addressof, std::construct_at, std::destroy_at, std::destroy
#include <type_traits> // std::is_trivially_default_constructible, is_trivially_destructible, std::conditional

namespace orion
{
    template<typename T, std::size_t Capacity>
    class UninitializedStorage
    {
    public:
        static consteval bool is_trivial_storage() noexcept
        {
            return std::is_trivially_default_constructible_v<T> &&
                   std::is_trivially_destructible_v<T>;
        }

        using pointer = T*;
        using const_pointer = const T*;
        using storage_type = std::conditional_t<is_trivial_storage(),
                                                std::array<T, Capacity>,
                                                std::byte[sizeof(T) * Capacity]>;

        [[nodiscard]] constexpr pointer data() noexcept
        {
            if constexpr (is_trivial_storage()) {
                return static_cast<pointer>(storage_.data());
            } else {
                return reinterpret_cast<pointer>(std::addressof(storage_));
            }
        }
        [[nodiscard]] constexpr const_pointer data() const noexcept
        {
            if constexpr (is_trivial_storage()) {
                return static_cast<const_pointer>(storage_.data());
            } else {
                return reinterpret_cast<const_pointer>(std::addressof(storage_));
            }
        }

    private:
        alignas(alignof(T)) storage_type storage_;
    };

    template<std::forward_iterator ForwardIt>
    constexpr void uninitialized_default_construct(ForwardIt first, ForwardIt last)
    {
        ForwardIt current = first;
        try {
            for (; current != last; ++current) {
                std::construct_at(std::addressof(*current));
            }
        } catch (...) {
            std::destroy(first, last);
            throw;
        }
    }

    template<std::forward_iterator ForwardIt, typename T>
    constexpr void uninitialized_fill(ForwardIt first, ForwardIt last, const T& value)
    {
        ForwardIt current = first;
        try {
            for (; current != last; ++current) {
                std::construct_at(std::addressof(*current), value);
            }
        } catch (...) {
            for (; first != current; ++first) {
                std::destroy_at(std::addressof(*first));
            }
            throw;
        }
    }

    template<std::input_iterator InputIt, std::forward_iterator ForwardIt>
    constexpr ForwardIt uninitialized_copy(InputIt first, InputIt last, ForwardIt out_first)
    {
        ForwardIt current = out_first;
        try {
            for (; first != last; ++first, (void)++current) {
                std::construct_at(std::addressof(*current), *first);
            }
            return current;
        } catch (...) {
            for (; out_first != current; ++out_first) {
                std::destroy_at(std::addressof(*out_first));
            }
            throw;
        }
    }

    template<std::input_iterator InputIt, std::forward_iterator ForwardIt>
    constexpr ForwardIt uninitialized_move(InputIt first, InputIt last, ForwardIt out_first)
    {
        ForwardIt current = out_first;
        try {
            for (; first != last; ++first, (void)++current) {
                std::construct_at(std::addressof(*current), std::move(*first));
            }
            return current;
        } catch (...) {
            std::destroy(out_first, current);
            throw;
        }
    }
} // namespace orion
