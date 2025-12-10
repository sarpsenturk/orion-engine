#pragma once

#include "orion/assert.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace orion
{
    template<typename T, std::size_t N>
    struct Vector {
        std::array<T, N> values_;

        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;

        // Size

        static constexpr auto empty = std::bool_constant<N == 0>{};
        static constexpr auto size = std::integral_constant<std::size_t, N>{};

        // Element access

        reference at(size_type idx)
        {
            if (idx >= size()) {
                throw std::out_of_range("vector element index out of range");
            }
            return values_[idx];
        }

        const_reference at(size_type idx) const
        {
            if (idx >= size()) {
                throw std::out_of_range("vector element index out of range");
            }
            return values_[idx];
        }

        reference operator[](size_type idx)
        {
            ORION_ASSERT(idx < N, "vector element index out of range");
            return values_[idx];
        }
        const_reference operator[](size_type idx) const
        {
            ORION_ASSERT(idx < N, "vector element index out of range");
            return values_[idx];
        }

        pointer data() { return values_.data(); }
        const_pointer data() const { return values_.data(); }

        // Comparison

        friend bool operator==(const Vector&, const Vector&) = default;

        // Vector operations

        friend Vector operator+(const Vector& lhs, const Vector& rhs)
        {
            Vector result;
            for (size_type i = 0; i < size(); ++i) {
                result[i] = lhs[i] + rhs[i];
            }
            return result;
        }
        friend Vector& operator+=(Vector& lhs, const Vector& rhs)
        {
            for (size_type i = 0; i < size(); ++i) {
                lhs[i] += rhs[i];
            }
            return lhs;
        }

        friend Vector operator-(const Vector& lhs, const Vector& rhs)
        {
            Vector result;
            for (size_type i = 0; i < size(); ++i) {
                result[i] = lhs[i] - rhs[i];
            }
            return result;
        }
        friend Vector& operator-=(Vector& lhs, const Vector& rhs)
        {
            for (size_type i = 0; i < size(); ++i) {
                lhs[i] -= rhs[i];
            }
            return lhs;
        }

        friend Vector operator-(const Vector& vector)
        {
            Vector result;
            for (size_type i = 0; i < size(); ++i) {
                result[i] = -vector[i];
            }
            return result;
        }

        template<typename R, typename result_type = std::common_type_t<value_type, R>>
        friend Vector<result_type, N> operator*(const Vector& vector, R scalar)
            requires(std::is_arithmetic_v<R>)
        {
            Vector<result_type, N> result;
            for (size_type i = 0; i < size(); ++i) {
                result[i] = static_cast<result_type>(vector[i]) * scalar;
            }
            return result;
        }
        friend Vector& operator*=(Vector& vector, auto scalar)
            requires(std::is_arithmetic_v<decltype(scalar)>)
        {
            for (size_type i = 0; i < size(); ++i) {
                vector[i] *= scalar;
            }
            return vector;
        }

        template<typename R, typename result_type = std::common_type_t<value_type, R>>
        friend Vector<result_type, N> operator/(const Vector& vector, R scalar)
            requires(std::is_arithmetic_v<R>)
        {
            Vector<result_type, N> result;
            for (size_type i = 0; i < size(); ++i) {
                result[i] = static_cast<result_type>(vector[i]) / scalar;
            }
            return result;
        }
        friend Vector& operator/=(Vector& vector, auto scalar)
            requires(std::is_arithmetic_v<decltype(scalar)>)
        {
            for (size_type i = 0; i < size(); ++i) {
                vector[i] /= scalar;
            }
            return vector;
        }

        friend value_type dot(const Vector& a, const Vector& b)
        {
            value_type result = {};
            for (size_type i = 0; i < size(); ++i) {
                result += a[i] * b[i];
            }
            return result;
        }

        friend Vector cross(const Vector& lhs, const Vector& rhs)
            requires(N == 3) // only defined in 3d
        {
            Vector result;
            result[0] = lhs[1] * rhs[2] - lhs[2] * rhs[1];
            result[1] = lhs[2] * rhs[0] - lhs[0] * rhs[2];
            result[2] = lhs[0] * rhs[1] - lhs[1] * rhs[0];
            return result;
        }

        // Magnitude and normalization

        value_type sqr_magnitude() const
        {
            value_type result = {};
            for (size_type i = 0; i < size(); ++i) {
                result += values_[i] * values_[i];
            }
            return result;
        }

        template<typename Result = std::conditional_t<std::is_floating_point_v<value_type>, value_type, float>>
        Result magnitude() const
        {
            return std::sqrt(static_cast<Result>(sqr_magnitude()));
        }

        template<typename Result = std::conditional_t<std::is_floating_point_v<value_type>, value_type, float>>
        friend Vector<Result, N> normalize(const Vector& vector)
        {
            Vector<Result, N> result;
            const auto mag = vector.magnitude<Result>();
            for (size_type i = 0; i < size(); ++i) {
                result[i] = vector.values_[i] / mag;
            }
            return result;
        }
    };

    // Template deduction guide
    template<typename T, typename... Ts>
    Vector(T, Ts&&...) -> Vector<T, sizeof...(Ts) + 1>;

    // Common vector type definitions

    using Vector2i = Vector<int, 2>;
    using Vector2u = Vector<std::uint32_t, 2>;
    using Vector2f = Vector<float, 2>;
    using Vector2d = Vector<double, 2>;

    using Vector3i = Vector<int, 3>;
    using Vector3u = Vector<std::uint32_t, 3>;
    using Vector3f = Vector<float, 3>;
    using Vector3d = Vector<double, 3>;

    using Vector4i = Vector<int, 4>;
    using Vector4u = Vector<std::uint32_t, 4>;
    using Vector4f = Vector<float, 4>;
    using Vector4d = Vector<double, 4>;
} // namespace orion