#pragma once

#include "orion-utils/assertion.h"     // ORION_ASSERT
#include "orion-utils/uninitialized.h" // orion::UninitializedArray, orion::uninitialized_default_construct

#include <algorithm>        // std::move_backwards, std::move, std::equal
#include <cstddef>          // std::size_t, std::ptrdiff_t
#include <initializer_list> // std::initializer_list
#include <iterator>         // std::reverse_iterator
#include <limits>           // std::numeric_limits
#include <memory>           // std::destroy_n
#include <type_traits>      // std::make_signed, std::is_*

namespace orion
{
    namespace detail
    {
        template<typename T, std::size_t Capacity>
        class StaticVector
        {
        public:
            static consteval auto find_min_size_type() noexcept
            {
                if constexpr (Capacity <= std::numeric_limits<std::uint8_t>::max()) {
                    return std::uint8_t{};
                } else if constexpr (Capacity <= std::numeric_limits<std::uint16_t>::max()) {
                    return std::uint16_t{};
                } else if constexpr (Capacity <= std::numeric_limits<std::uint32_t>::max()) {
                    return std::uint32_t{};
                } else if constexpr (Capacity <= std::numeric_limits<std::uint64_t>::max()) {
                    return std::uint64_t{};
                } else {
                    return std::uintmax_t{};
                }
            }

            using value_type = T;
            using reference = value_type&;
            using const_reference = const value_type&;
            using pointer = value_type*;
            using const_pointer = const value_type*;
            using size_type = decltype(find_min_size_type());
            using difference_type = std::make_signed_t<size_type>;
            using iterator = pointer;
            using const_iterator = const_pointer;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;

            constexpr StaticVector() = default;

            constexpr explicit StaticVector(size_type n)
                requires std::is_default_constructible_v<value_type>
                : size_(n)
            {
                ORION_ASSERT(size() <= max_size());
                orion::uninitialized_default_construct(begin(), begin() + n);
            }

            constexpr StaticVector(size_type n, const_reference value)
                requires std::is_copy_constructible_v<value_type>
                : size_(n)
            {
                ORION_ASSERT(size() <= max_size());
                orion::uninitialized_fill(begin(), begin() + n, value);
            }

            template<std::input_iterator InputIt>
            constexpr StaticVector(InputIt first, InputIt last)
                : size_(static_cast<size_type>(std::distance(first, last)))
            {
                ORION_ASSERT(size() <= max_size());
                orion::uninitialized_copy(first, last, begin());
            }

            constexpr StaticVector(const StaticVector& other) noexcept(std::is_nothrow_copy_constructible_v<value_type>)
                : size_(other.size_)
            {
                orion::uninitialized_copy(other.begin(), other.end(), begin());
            }

            constexpr StaticVector(StaticVector&& other) noexcept(std::is_nothrow_move_constructible_v<value_type>)
                : size_(other.size_)
            {
                orion::uninitialized_move(other.begin(), other.end(), begin());
            }

            constexpr StaticVector& operator=(const StaticVector& other) noexcept(std::is_nothrow_copy_assignable_v<value_type>)
            {
                if (&other != this) {
                    clear();
                    orion::uninitialized_copy(other.begin(), other.end(), begin());
                    size_ = other.size_;
                }
                return *this;
            }

            constexpr StaticVector& operator=(StaticVector&& other) noexcept(std::is_nothrow_move_assignable_v<value_type>)
            {
                if (&other != this) {
                    clear();
                    orion::uninitialized_move(other.begin(), other.end(), begin());
                    size_ = other.size_;
                }
                return *this;
            }

            constexpr ~StaticVector()
                requires std::is_trivially_destructible_v<value_type>
            = default;

            constexpr ~StaticVector()
            {
                std::destroy_n(begin(), size_);
            }

            [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
            [[nodiscard]] constexpr size_type size() const noexcept { return size_; }
            [[nodiscard]] static constexpr size_type max_size() noexcept { return Capacity; }
            [[nodiscard]] static constexpr size_type capacity() noexcept { return Capacity; }

            [[nodiscard]] constexpr pointer data() noexcept { return elements_.data(); }
            [[nodiscard]] constexpr const_pointer data() const noexcept { return elements_.data(); }

            [[nodiscard]] constexpr iterator begin() noexcept { return data(); }
            [[nodiscard]] constexpr const_iterator begin() const noexcept { return data(); }
            [[nodiscard]] constexpr iterator end() noexcept { return data() + size_; }
            [[nodiscard]] constexpr const_iterator end() const noexcept { return data() + size_; }
            [[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return data() + size_ - 1; }
            [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return data() + size_ - 1; }
            [[nodiscard]] constexpr reverse_iterator rend() noexcept { return data() - 1; }
            [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return data() - 1; }
            [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return data(); }
            [[nodiscard]] constexpr const_iterator cend() const noexcept { return data() + size_; }
            [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return data() + size_ - 1; }
            [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return data() - 1; }

            [[nodiscard]] constexpr reference operator[](size_type n)
            {
                ORION_ASSERT(n < max_size());
                return *(begin() + n);
            }
            [[nodiscard]] constexpr const_reference operator[](size_type n) const
            {
                ORION_ASSERT(n < max_size());
                return *(begin() + n);
            }
            [[nodiscard]] constexpr reference front()
            {
                ORION_ASSERT(!empty());
                return *begin();
            }
            [[nodiscard]] constexpr const_reference front() const
            {
                ORION_ASSERT(!empty());
                return *begin();
            }
            [[nodiscard]] constexpr reference back()
            {
                ORION_ASSERT(!empty());
                return *(end() - 1);
            }
            [[nodiscard]] constexpr const_reference back() const
            {
                ORION_ASSERT(!empty());
                return *(end() - 1);
            }

            constexpr void clear() noexcept
            {
                if constexpr (!std::is_trivially_destructible_v<value_type>) {
                    std::destroy_n(begin(), size_);
                }
                size_ = 0;
            }

            template<typename... Args>
            constexpr iterator emplace(const_iterator position, Args&&... args)
            {
                ORION_ASSERT(size() < max_size());
                ORION_ASSERT(position <= end());
                auto where = const_cast<iterator>(position);
                std::move_backward(where, end(), end() + 1);
                std::construct_at(std::addressof(*position), std::forward<Args>(args)...);
                ++size_;
                return where;
            }
            template<typename... Args>
            constexpr iterator emplace_back(Args&&... args)
            {
                return emplace(end(), std::forward<Args>(args)...);
            }

            constexpr void push_back(const_reference value)
            {
                emplace(end(), value);
            }
            constexpr void push_back(value_type&& value)
            {
                emplace(end(), std::move(value));
            }

            constexpr iterator insert(const_iterator position, const_reference value)
            {
                return emplace(position, value);
            }
            constexpr iterator insert(const_iterator position, value_type&& value)
            {
                return emplace(position, std::move(value));
            }
            template<std::input_iterator InputIt>
            constexpr iterator insert(const_iterator position, InputIt first, InputIt last)
            {
                iterator inserted = nullptr;
                for (; first != last; ++first, (void)++position) {
                    inserted = emplace(position, *first);
                }
                return inserted;
            }
            constexpr iterator insert(const_iterator position, std::initializer_list<value_type> list)
            {
                iterator inserted = nullptr;
                for (auto iter = list.begin(); iter != list.end(); ++iter, (void)++position) {
                    inserted = emplace(position, *iter);
                }
                return inserted;
            }

            constexpr iterator erase(const_iterator position)
            {
                ORION_ASSERT(position <= end());
                auto where = const_cast<iterator>(position);
                std::destroy_at(std::addressof(*where));
                std::move(where + 1, end(), where);
                --size_;
                return where;
            }
            constexpr iterator erase(const_iterator first, const_iterator last)
            {
                ORION_ASSERT(last <= end());
                auto where = const_cast<iterator>(first);
                size_type count = 0;
                for (; first != last; ++first, (void)++count) {
                    std::destroy_at(std::addressof(*first));
                }
                std::move(const_cast<iterator>(last), end(), where);
                size_ -= count;
                return where;
            }

            constexpr void pop_back()
            {
                erase(end() - 1);
            }

            [[nodiscard]] constexpr friend bool operator==(const StaticVector& lhs, const StaticVector& rhs)
            {
                return std::equal(lhs.begin(), lhs.end(), rhs.begin());
            }

        private:
            UninitializedStorage<value_type, Capacity> elements_{};
            size_type size_ = 0;
        };
    } // namespace detail

    template<typename T, std::size_t Capacity>
    using static_vector = detail::StaticVector<T, Capacity>;
} // namespace orion
