#pragma once

#include "orion/assertion.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <stdexcept>

namespace orion
{
    template<typename T, std::size_t M, std::size_t N>
    struct Matrix {
        // Public typedefs

        using value_type = T;
        using container_type = std::array<T, M * N>;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = typename container_type::iterator;
        using const_iterator = typename container_type::const_iterator;
        using reverse_iterator = std::reverse_iterator<iterator>;
        using const_reverse_iterator = std::reverse_iterator<const_iterator>;
        using size_type = std::size_t;

        // Internal data container
        container_type data_;

        // Row & Column accessors

        static constexpr std::integral_constant<std::size_t, M> rows = {};
        static constexpr std::integral_constant<std::size_t, N> cols = {};
        static constexpr std::bool_constant<M * N == 0> empty = {};

        // Iterators

        constexpr iterator begin() noexcept { return data_.begin(); }
        constexpr iterator end() noexcept { return data_.end(); }
        constexpr const_iterator begin() const noexcept { return data_.begin(); }
        constexpr const_iterator end() const noexcept { return data_.end(); }
        constexpr const_iterator cbegin() const noexcept { return data_.cbegin(); }
        constexpr const_iterator cend() const noexcept { return data_.cend(); }
        constexpr reverse_iterator rbegin() noexcept { return data_.rbegin(); }
        constexpr reverse_iterator rend() noexcept { return data_.rend(); }
        constexpr const_reverse_iterator rbegin() const noexcept { return data_.rbegin(); }
        constexpr const_reverse_iterator rend() const noexcept { return data_.rend(); }

        // Non-throwing element accessor

        constexpr reference operator()(std::size_t row, std::size_t col)
        {
            ORION_ASSERT(row < M);
            ORION_ASSERT(col < N);
            return data_[row * N + col];
        }
        constexpr const_reference operator()(std::size_t row, std::size_t col) const
        {
            ORION_ASSERT(row < M);
            ORION_ASSERT(col < N);
            return data_[row * N + col];
        }

        // Throwing element accessor

        constexpr reference at(std::size_t row, std::size_t col)
        {
            if (row >= M || col >= N) {
                throw std::out_of_range("Matrix::at out of range");
            }
            return data_[row * N + col];
        }
        constexpr const const_reference at(std::size_t row, std::size_t col) const
        {
            if (row >= M || col >= N) {
                throw std::out_of_range("Matrix::at out of range");
            }
            return data_[row * N + col];
        }

        // Identity matrix
        static consteval Matrix identity()
            requires(M == N)
        {
            Matrix result{}; // zero initialize
            for (std::size_t i = 0; i < M; i++) {
                result(i, i) = T{1};
            }
            return result;
        }

        // Equality operator

        constexpr friend bool operator==(const Matrix& lhs, const Matrix& rhs) = default;

        // Addition

        constexpr friend Matrix operator+(const Matrix& lhs, const Matrix& rhs)
        {
            Matrix result;
            std::transform(lhs.begin(), lhs.end(), rhs.begin(), result.begin(), [](auto lhs, auto rhs) { return lhs + rhs; });
            return result;
        }

        // Subtraction

        constexpr friend Matrix operator-(const Matrix& lhs, const Matrix& rhs)
        {
            Matrix result;
            std::transform(lhs.begin(), lhs.end(), rhs.begin(), result.begin(), [](auto lhs, auto rhs) { return lhs - rhs; });
            return result;
        }

        // Scalar multiplication

        constexpr friend auto operator*(const Matrix& matrix, auto scalar) -> Matrix<std::common_type_t<T, decltype(scalar)>, M, N>
            requires(std::integral<decltype(scalar)> || std::floating_point<decltype(scalar)>)
        {
            Matrix<std::common_type_t<T, decltype(scalar)>, M, N> result;
            std::transform(matrix.begin(), matrix.end(), result.begin(), [scalar](auto lhs) { return lhs * scalar; });
            return result;
        }

        constexpr friend auto operator*(auto scalar, const Matrix& matrix) -> Matrix<std::common_type_t<T, decltype(scalar)>, M, N>
            requires(std::integral<decltype(scalar)> || std::floating_point<decltype(scalar)>)
        {
            Matrix<std::common_type_t<T, decltype(scalar)>, M, N> result;
            std::transform(matrix.begin(), matrix.end(), result.begin(), [scalar](auto lhs) { return lhs * scalar; });
            return result;
        }

        // Transposition

        constexpr Matrix<T, N, M> transpose() const
        {
            Matrix<T, N, M> result;
            for (std::size_t row = 0; row < M; ++row) {
                for (std::size_t col = 0; col < N; ++col) {
                    result(col, row) = (*this)(row, col);
                }
            }
            return result;
        }

        // Matrix multiplication

        template<typename U, std::size_t P>
        constexpr friend Matrix<std::common_type_t<T, U>, M, P> operator*(const Matrix& lhs, const Matrix<U, N, P>& rhs)
        {
            Matrix<std::common_type_t<T, U>, M, P> result;
            for (std::size_t i = 0; i < M; ++i) {
                for (std::size_t j = 0; j < P; ++j) {
                    auto sum = std::common_type_t<T, U>{0};
                    for (std::size_t k = 0; k < N; ++k) {
                        sum += lhs(i, k) * rhs(k, j);
                    }
                    result(i, j) = sum;
                }
            }
            return result;
        }
    };
} // namespace orion
