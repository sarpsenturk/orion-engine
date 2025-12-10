#pragma once

#include "orion/math/vector.hpp"

#include "orion/assert.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

namespace orion
{
    template<typename T, std::size_t M, std::size_t N>
    struct Matrix {
        std::array<T, M * N> values_;

        using value_type = T;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;

        // Size

        static constexpr auto rows = std::integral_constant<size_type, M>{};
        static constexpr auto cols = std::integral_constant<size_type, N>{};

        // Identity

        static Matrix identity()
            requires(M == N) // only defined on square matrices
        {
            Matrix identity = {};
            for (size_type i = 0; i < M; ++i) {
                identity(i, i) = value_type{1};
            }
            return identity;
        }

        // Element access

        reference at(size_type row, size_type col)
        {
            if (row >= rows || col >= cols) {
                throw std::out_of_range("matrix element position out of range");
            }
            return values_[row * cols + col];
        }
        const_reference at(size_type row, size_type col) const
        {
            if (row >= rows || col >= cols) {
                throw std::out_of_range("matrix element position out of range");
            }
            return values_[row * cols + col];
        }

        reference operator()(size_type row, size_type col)
        {
            ORION_ASSERT((row < rows && col < cols), "matrix element position out of range");
            return values_[row * cols + col];
        }
        const_reference operator()(size_type row, size_type col) const
        {
            ORION_ASSERT((row < rows && col < cols), "matrix element position out of range");
            return values_[row * cols + col];
        }

        pointer data() { return values_.data(); }
        const_pointer data() const { return values_.data(); }

        // Comparison

        friend bool operator==(const Matrix&, const Matrix&) = default;

        // Matrix operations

        friend Matrix operator+(const Matrix& lhs, const Matrix& rhs)
        {
            Matrix result;
            for (size_type i = 0; i < rows * cols; ++i) {
                result.values_[i] = lhs.values_[i] + rhs.values_[i];
            }
            return result;
        }
        friend Matrix& operator+(Matrix& lhs, const Matrix& rhs)
        {
            for (size_type i = 0; i < rows * cols; ++i) {
                lhs.values_[i] += rhs.values_[i];
            }
            return lhs;
        }

        friend Matrix operator-(const Matrix& lhs, const Matrix& rhs)
        {
            Matrix result;
            for (size_type i = 0; i < rows * cols; ++i) {
                result.values_[i] = lhs.values_[i] - rhs.values_[i];
            }
            return result;
        }
        friend Matrix& operator-(Matrix& lhs, const Matrix& rhs)
        {
            for (size_type i = 0; i < rows * cols; ++i) {
                lhs.values_[i] -= rhs.values_[i];
            }
            return lhs;
        }

        template<typename R, typename result_type = std::common_type_t<value_type, R>>
        friend Matrix<result_type, M, N> operator*(const Matrix& matrix, R scalar)
            requires(std::is_arithmetic_v<R>)
        {
            Matrix<result_type, M, N> result;
            for (size_type i = 0; i < rows * cols; ++i) {
                result.values_[i] = static_cast<result_type>(matrix.values_[i]) * scalar;
            }
            return result;
        }
        friend Matrix operator*=(Matrix& matrix, auto scalar)
            requires(std::is_arithmetic_v<decltype(scalar)>)
        {
            for (size_type i = 0; i < rows * cols; ++i) {
                matrix.values_[i] *= scalar;
            }
            return matrix;
        }

        template<typename R, typename result_type = std::common_type_t<value_type, R>>
        friend Matrix<result_type, M, N> operator/(const Matrix& matrix, R scalar)
            requires(std::is_arithmetic_v<R>)
        {
            Matrix<result_type, M, N> result;
            for (size_type i = 0; i < rows * cols; ++i) {
                result.values_[i] = static_cast<result_type>(matrix.values_[i]) / scalar;
            }
            return result;
        }
        friend Matrix operator/=(Matrix& matrix, auto scalar)
            requires(std::is_arithmetic_v<decltype(scalar)>)
        {
            for (size_type i = 0; i < rows * cols; ++i) {
                matrix.values_[i] /= scalar;
            }
            return matrix;
        }

        template<typename R, std::size_t K, typename result_type = std::common_type_t<value_type, R>>
        friend Matrix<result_type, M, K> operator*(const Matrix& lhs, const Matrix<R, N, K>& rhs)
        {
            Matrix<result_type, M, K> result;
            for (size_type i = 0; i < M; ++i) {
                for (size_type j = 0; j < K; ++j) {
                    result_type sum = {};
                    for (size_type k = 0; k < N; ++k) {
                        sum += lhs(i, k) * rhs(k, j);
                    }
                    result(i, j) = sum;
                }
            }
            return result;
        }

        template<typename R, typename result_type = std::common_type_t<value_type, R>>
        friend Vector<result_type, N> operator*(const Matrix& matrix, const Vector<R, N>& vector)
        {
            Vector<result_type, N> result;
            for (size_type i = 0; i < M; ++i) {
                result_type sum = {};
                for (size_type j = 0; j < N; ++j) {
                    sum += matrix(i, j) * vector[j];
                }
                result[i] = sum;
            }
            return result;
        }

        // Row-column access

        Vector<value_type, N> row(size_type idx) const
        {
            if (idx >= rows) {
                throw std::out_of_range("row index out of range");
            }
            Vector<value_type, N> result;
            for (size_type i = 0; i < cols; ++i) {
                result[i] = (*this)(idx, i);
            }
            return result;
        }

        Vector<value_type, M> column(size_type idx) const
        {
            if (idx >= cols) {
                throw std::out_of_range("column index out of range");
            }
            Vector<value_type, M> result;
            for (size_type i = 0; i < rows; ++i) {
                result[i] = (*this)(i, idx);
            }
            return result;
        }

        // Transpose

        Matrix<value_type, N, M> transpose() const
        {
            Matrix<value_type, N, M> result;
            for (size_type i = 0; i < rows; ++i) {
                for (size_type j = 0; j < cols; ++j) {
                    result(j, i) = (*this)(i, j);
                }
            }
            return result;
        }
    };

    // Common matrix type definitions

    using Matrix2i = Matrix<int, 2, 2>;
    using Matrix2u = Matrix<std::uint32_t, 2, 2>;
    using Matrix2f = Matrix<float, 2, 2>;
    using Matrix2d = Matrix<double, 2, 2>;

    using Matrix3i = Matrix<int, 3, 3>;
    using Matrix3u = Matrix<std::uint32_t, 3, 3>;
    using Matrix3f = Matrix<float, 3, 3>;
    using Matrix3d = Matrix<double, 3, 3>;

    using Matrix4i = Matrix<int, 4, 4>;
    using Matrix4u = Matrix<std::uint32_t, 4, 4>;
    using Matrix4f = Matrix<float, 4, 4>;
    using Matrix4d = Matrix<double, 4, 4>;
} // namespace orion