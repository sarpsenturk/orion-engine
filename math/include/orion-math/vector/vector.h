#pragma once

#include "orion-math/concepts.h" // arithmetic
#include "orion-math/func.h"     // negate, plus, minus
#include "orion-math/sqrt.h"     // orion::math::sqrt

#include <algorithm>   // std::ranges::transform, std::ranges::for_each, std::accumulate
#include <array>       // std::array
#include <concepts>    // std::floating_point
#include <cstddef>     // std::size_t, std::ptrdiff_t
#include <iterator>    // std::prev
#include <numeric>     // std::transform_reduce, std::inner_product
#include <ranges>      // std::ranges::input_range, std::ranges::begin, std::ranges::end
#include <stdexcept>   // std::out_of_range
#include <type_traits> // std::common_type, std::is_same_v

#define ORION_VECTOR_DEFINE_COMPONENT(name, index)                \
    [[nodiscard]] constexpr reference name() noexcept             \
        requires(index < N)                                       \
    {                                                             \
        return components_[index];                                \
    }                                                             \
    [[nodiscard]] constexpr const_reference name() const noexcept \
        requires(index < N)                                       \
    {                                                             \
        return components_[index];                                \
    }

namespace orion::math
{
    template<arithmetic T, std::size_t N>
    struct Vector {
    public:
        using value_type = T;
        using storage = std::array<value_type, N>;
        using reference = value_type&;
        using const_reference = const value_type&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using iterator = typename storage::iterator;
        using const_iterator = typename storage::const_iterator;
        using reverse_iterator = typename storage ::reverse_iterator;
        using const_reverse_iterator = typename storage ::const_reverse_iterator;

        [[nodiscard]] constexpr bool is_empty() const noexcept { return N == 0; }

        [[nodiscard]] constexpr size_type size() const noexcept { return N; }
        [[nodiscard]] constexpr size_type max_size() const noexcept { return N; }

        [[nodiscard]] constexpr reference at(size_type pos)
        {
            if (pos >= size()) {
                throw std::out_of_range("position out of range of vector");
            }
            return components_[pos];
        }
        [[nodiscard]] constexpr const_reference at(size_type pos) const
        {
            if (pos >= size()) {
                throw std::out_of_range("position out of range of vector");
            }
            return components_[pos];
        }

        [[nodiscard]] constexpr reference operator[](size_type pos) noexcept { return components_[pos]; }
        [[nodiscard]] constexpr const_reference operator[](size_type pos) const noexcept { return components_[pos]; }

        ORION_VECTOR_DEFINE_COMPONENT(x, 0)
        ORION_VECTOR_DEFINE_COMPONENT(y, 1)
        ORION_VECTOR_DEFINE_COMPONENT(z, 2)
        ORION_VECTOR_DEFINE_COMPONENT(w, 3)

        [[nodiscard]] constexpr auto sqr_magnitude() const noexcept
        {
            return std::inner_product(begin(), end(), begin(), value_type{});
        }

        [[nodiscard]] constexpr auto magnitude() const noexcept
        {
            return sqrt(sqr_magnitude());
        }

        [[nodiscard]] constexpr auto normalized() const noexcept
        {
            return *this / magnitude();
        }

        constexpr Vector& normalize() noexcept
            requires std::floating_point<value_type>
        {
            *this = normalized();
            return *this;
        }

        [[nodiscard]] constexpr reference front() noexcept { return *begin(); }
        [[nodiscard]] constexpr const_reference front() const noexcept { return *begin(); }

        [[nodiscard]] constexpr reference back() noexcept { return *std::prev(end()); }
        [[nodiscard]] constexpr const_reference back() const noexcept { return *std::prev(end()); }

        [[nodiscard]] constexpr pointer data() noexcept { return components_.data(); }
        [[nodiscard]] constexpr const_pointer data() const noexcept { return components_.data(); }

        [[nodiscard]] constexpr iterator begin() noexcept { return components_.begin(); }
        [[nodiscard]] constexpr const_iterator begin() const noexcept { return components_.begin(); }
        [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return components_.cbegin(); }

        [[nodiscard]] constexpr iterator end() noexcept { return components_.end(); }
        [[nodiscard]] constexpr const_iterator end() const noexcept { return components_.end(); }
        [[nodiscard]] constexpr const_iterator cend() const noexcept { return components_.cend(); }

        [[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return components_.rbegin(); }
        [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return components_.rbegin(); }
        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return components_.crbegin(); }

        [[nodiscard]] constexpr reverse_iterator rend() noexcept { return components_.rend(); }
        [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return components_.rend(); }
        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return components_.crend(); }

        [[nodiscard]] friend constexpr bool operator==(const Vector& lhs, const Vector& rhs) noexcept = default;

        [[nodiscard]] friend constexpr auto operator*(const Vector& vector, arithmetic auto scalar) noexcept
        {
            using common_type = std::common_type_t<value_type, decltype(scalar)>;
            using ResultVector = Vector<common_type, N>;
            ResultVector result;
            std::ranges::transform(vector, result.begin(), [&scalar](auto component) { return component * scalar; });
            return result;
        }

        [[nodiscard]] friend constexpr auto operator/(const Vector& vector, arithmetic auto scalar) noexcept
        {
            using common_type = std::common_type_t<value_type, decltype(scalar)>;
            using ResultVector = Vector<common_type, N>;
            ResultVector result;
            std::ranges::transform(vector, result.begin(), [&scalar](auto component) { return component / scalar; });
            return result;
        }

        [[nodiscard]] friend constexpr Vector operator-(const Vector& vector) noexcept
        {
            Vector result;
            std::ranges::transform(vector, result.begin(), negate<>{});
            return result;
        }

        [[nodiscard]] friend constexpr Vector operator+(const Vector& lhs, const Vector& rhs) noexcept
        {
            Vector result;
            std::ranges::transform(lhs, rhs, result.begin(), plus<>{});
            return result;
        }

        [[nodiscard]] friend constexpr Vector operator-(const Vector& lhs, const Vector& rhs) noexcept
        {
            Vector result;
            std::ranges::transform(lhs, rhs, result.begin(), minus<>{});
            return result;
        }

        storage components_; // NOLINT(misc-non-private-member-variables-in-classes)
    };

    template<typename T, std::size_t N>
    [[nodiscard]] constexpr auto dot(const Vector<T, N>& lhs, const Vector<T, N>& rhs) noexcept
    {
        using value_type = typename Vector<T, N>::value_type;
        return std::inner_product(lhs.begin(), lhs.end(), rhs.begin(), value_type{});
    }

    template<typename T>
    [[nodiscard]] constexpr Vector<T, 3> cross(const Vector<T, 3>& lhs, const Vector<T, 3>& rhs) noexcept
    {
        return {
            lhs[1] * rhs[2] - lhs[2] * rhs[1],
            lhs[2] * rhs[0] - lhs[0] * rhs[2],
            lhs[0] * rhs[1] - lhs[1] * rhs[0]};
    }

    template<typename T, typename T1, std::size_t N1>
    [[nodiscard]] constexpr auto vector_cast(const Vector<T1, N1>& vector) noexcept
        requires std::convertible_to<T1, T>
    {
        if constexpr (std::is_same_v<T1, T>) {
            return vector;
        } else {
            Vector<T, N1> result;
            std::ranges::transform(vector, result.begin(), [](const T1& component) { return static_cast<T>(component); });
            return result;
        }
    }

    template<typename T, typename... Other>
    Vector(T first, Other... other) -> Vector<T, sizeof...(Other) + 1>;
} // namespace orion::math

#undef ORION_VECTOR_DEFINE_COMPONENT
