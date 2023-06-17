#pragma once

#include "orion-math/concepts.h"      // arithmetic
#include "orion-math/func.h"          // negate, plus, minus
#include "orion-math/vector/vector.h" // vector

#include <algorithm> // std::transform
#include <array>     // std::array
#include <cstddef>   // std::size_t
#include <stdexcept> // std::out_of_range

namespace orion::math
{
    template<typename T, std::size_t Rows, std::size_t Cols>
    struct Matrix {
        using value_type = T;
        using row_type = Vector<T, Cols>;
        using storage = std::array<row_type, Rows>;
        using reference = value_type&;
        using const_reference = const value_type&;
        using pointer = value_type*;
        using const_pointer = const value_type*;
        using size_type = std::size_t;
        using iterator = typename storage::iterator;
        using const_iterator = typename storage::const_iterator;
        using reverse_iterator = typename storage::reverse_iterator;
        using const_reverse_iterator = typename storage::const_reverse_iterator;

        static constexpr auto rows = Rows;
        static constexpr auto columns = Cols;

        storage elements_;

        [[nodiscard]] static constexpr Matrix identity() noexcept
            requires(rows == columns)
        {
            Matrix identity{};
            for (std::size_t i = 0; i < rows; ++i) {
                identity[i][i] = value_type{1};
            }
            return identity;
        }

        [[nodiscard]] static constexpr size_type size() noexcept { return rows * columns; }
        [[nodiscard]] static constexpr bool is_empty() noexcept { return size() == 0; }

        [[nodiscard]] constexpr pointer data() noexcept { return elements_.data(); }
        [[nodiscard]] constexpr const_pointer data() const noexcept { return elements_.data(); }

        [[nodiscard]] constexpr row_type& operator[](size_type idx) noexcept { return elements_[idx]; }
        [[nodiscard]] constexpr const row_type& operator[](size_type idx) const noexcept { return elements_[idx]; }

        [[nodiscard]] constexpr friend bool operator==(const Matrix& lhs, const Matrix& rhs) = default;

        [[nodiscard]] constexpr friend Matrix operator-(const Matrix& matrix) noexcept
        {
            Matrix result;
            std::ranges::transform(matrix, std::ranges::begin(result), negate<>{});
            return result;
        }

        [[nodiscard]] constexpr friend Matrix operator+(const Matrix& lhs, const Matrix& rhs) noexcept
        {
            Matrix result;
            std::ranges::transform(lhs, rhs, std::ranges::begin(result), plus<>{});
            return result;
        }
        [[nodiscard]] constexpr friend Matrix operator-(const Matrix& lhs, const Matrix& rhs) noexcept
        {
            Matrix result;
            std::ranges::transform(lhs, rhs, std::ranges::begin(result), minus<>{});
            return result;
        }

        [[nodiscard]] constexpr friend Matrix operator*(const Matrix& matrix, arithmetic auto scalar)
        {
            return scalar_multiply(matrix, scalar);
        }
        [[nodiscard]] constexpr friend Matrix operator*(arithmetic auto scalar, const Matrix& matrix)
        {
            return scalar_multiply(matrix, scalar);
        }

        template<typename T1, std::size_t Rows1, std::size_t Cols1>
        [[nodiscard]] constexpr friend auto operator*(const Matrix& lhs, const Matrix<T1, Rows1, Cols1>& rhs) noexcept
            requires(lhs.columns == rhs.rows)
        {
            using common_type = std::common_type_t<T, T1>;
            Matrix<common_type, Rows, Cols1> result;
            for (std::size_t i = 0; i < lhs.rows; ++i) {
                for (std::size_t j = 0; j < rhs.columns; ++j) {
                    common_type sum{};
                    for (std::size_t k = 0; k < lhs.columns; ++k) {
                        sum += lhs[i][k] * rhs[k][j];
                    }
                    result[i][j] = sum;
                }
            }
            return result;
        }

        [[nodiscard]] constexpr auto transpose() const noexcept -> Matrix<value_type, Cols, Rows>
        {
            Matrix<value_type, Cols, Rows> result;
            for (std::size_t i = 0; i < rows; ++i) {
                for (std::size_t j = 0; j < columns; ++j) {
                    result[j][i] = (*this)[i][j];
                }
            }
            return result;
        }

        [[nodiscard]] constexpr iterator begin() noexcept { return elements_.begin(); }
        [[nodiscard]] constexpr const_iterator begin() const noexcept { return elements_.begin(); }
        [[nodiscard]] constexpr const_iterator cbegin() const noexcept { return elements_.cbegin(); }

        [[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return elements_.rbegin(); }
        [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return elements_.rbegin(); }
        [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return elements_.crbegin(); }

        [[nodiscard]] constexpr iterator end() noexcept { return elements_.end(); }
        [[nodiscard]] constexpr const_iterator end() const noexcept { return elements_.end(); }
        [[nodiscard]] constexpr const_iterator cend() const noexcept { return elements_.cend(); }

        [[nodiscard]] constexpr reverse_iterator rend() noexcept { return elements_.rend(); }
        [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return elements_.rend(); }
        [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return elements_.crend(); }

    private:
        static constexpr Matrix scalar_multiply(const Matrix& matrix, arithmetic auto scalar)
        {
            Matrix result;
            std::ranges::transform(matrix, result.begin(), [&scalar](auto value) { return value * scalar; });
            return result;
        }
    };
} // namespace orion::math
