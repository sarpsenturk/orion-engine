#pragma once

#include "orion/math/sqrt.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <numeric>

namespace orion
{
    template<typename T, std::size_t N>
    struct Vector {
        [[no_unique_address]] std::array<T, N> data_;

        using value_type = T;
        using reference = T&;
        using const_reference = const T&;
        using iterator = typename std::array<T, N>::iterator;
        using const_iterator = typename std::array<T, N>::const_iterator;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using difference_type = std::ptrdiff_t;
        using size_type = std::size_t;

        static constexpr std::integral_constant<std::size_t, N> size = {};
        static constexpr std::integral_constant<std::size_t, N> max_size = {};
        static constexpr std::bool_constant<N == 0> empty = {};

        constexpr iterator begin() noexcept { return data_.begin(); }
        constexpr iterator end() noexcept { return data_.end(); }
        constexpr const_iterator begin() const noexcept { return data_.begin(); }
        constexpr const_iterator end() const noexcept { return data_.end(); }
        constexpr const_reverse_iterator rbegin() const noexcept { return data_.rbegin(); }
        constexpr const_reverse_iterator rend() const noexcept { return data_.rend(); }
        constexpr const_iterator cbegin() const noexcept { return data_.cbegin(); }
        constexpr const_iterator cend() const noexcept { return data_.cend(); }
        constexpr const_reverse_iterator crbegin() const noexcept { return data_.crbegin(); }
        constexpr const_reverse_iterator crend() const noexcept { return data_.crend(); }

        constexpr reference operator[](std::size_t index) { return data_[index]; }
        constexpr const_reference operator[](std::size_t index) const { return data_[index]; }

        constexpr reference at(std::size_t index) { return data_.at(index); }
        constexpr const_reference at(std::size_t index) const { return data_.at(index); }

        constexpr reference front() { return data_.front(); }
        constexpr const_reference front() const { return data_.front(); }

        constexpr reference back() { return data_.back(); }
        constexpr const_reference back() const { return data_.back(); }

        constexpr std::array<T, N>& as_array() noexcept { return data_; }
        constexpr const std::array<T, N>& as_array() const noexcept { return data_; }

        // Equality operators

        constexpr friend bool operator==(const Vector& lhs, const Vector& rhs)
        {
            return lhs.data_ == rhs.data_;
        }

        constexpr friend bool operator!=(const Vector& lhs, const Vector& rhs)
        {
            return lhs.data_ != rhs.data_;
        }

        // Addition

        constexpr friend Vector operator+(const Vector& lhs, const Vector& rhs)
        {
            Vector result;
            std::transform(lhs.begin(), lhs.end(), rhs.begin(), result.begin(), [](auto a, auto b) { return a + b; });
            return result;
        }

        constexpr friend Vector& operator+=(Vector& lhs, const Vector& rhs)
        {
            std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), [](auto a, auto b) { return a + b; });
            return lhs;
        }

        // Subtraction

        constexpr friend Vector operator-(const Vector& lhs, const Vector& rhs)
        {
            Vector result;
            std::transform(lhs.begin(), lhs.end(), rhs.begin(), result.begin(), [](auto a, auto b) { return a - b; });
            return result;
        }

        constexpr friend Vector& operator-=(Vector& lhs, const Vector& rhs)
        {
            std::transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), [](auto a, auto b) { return a - b; });
            return lhs;
        }

        // Scalar multiplication

        constexpr friend auto operator*(const Vector& vector, auto scalar) -> Vector<std::common_type_t<T, decltype(scalar)>, N>
        {
            Vector<std::common_type_t<T, decltype(scalar)>, N> result;
            std::transform(vector.begin(), vector.end(), result.begin(), [scalar](auto a) { return a * scalar; });
            return result;
        }

        constexpr friend auto operator*(auto scalar, const Vector& vector) -> Vector<std::common_type_t<T, decltype(scalar)>, N>
        {
            Vector<std::common_type_t<T, decltype(scalar)>, N> result;
            std::transform(vector.begin(), vector.end(), result.begin(), [scalar](auto a) { return a * scalar; });
            return result;
        }

        // Scalar division

        constexpr friend auto operator/(const Vector& vector, auto scalar) -> Vector<std::common_type_t<T, decltype(scalar)>, N>
        {
            Vector<std::common_type_t<T, decltype(scalar)>, N> result;
            std::transform(vector.begin(), vector.end(), result.begin(), [scalar](auto a) { return a / scalar; });
            return result;
        }

        // Vector magnitude
        constexpr T sqr_magnitude() const noexcept
        {
            return std::accumulate(data_.begin(), data_.end(), T{0}, [](auto a, auto b) { return a + b * b; });
        }

        template<std::floating_point R = float>
        constexpr R magnitude() const
        {
            return math::sqrt(static_cast<R>(sqr_magnitude()));
        }
    };

    template<typename T, typename... Ts>
    Vector(T, Ts...) -> Vector<T, sizeof...(Ts) + 1>;

    template<typename T, std::size_t N>
    constexpr T dot(const Vector<T, N>& lhs, const Vector<T, N>& rhs)
    {
        T result = 0;
        for (std::size_t i = 0; i < N; ++i) {
            result += lhs[i] * rhs[i];
        }
        return result;
    }
} // namespace orion
