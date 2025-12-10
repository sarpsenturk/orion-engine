#pragma once

#include "orion/math/matrix.hpp"
#include "orion/math/vector.hpp"

#include <cmath>

namespace orion
{
    template<typename T = float>
    Matrix<T, 4, 4> translate(const Matrix<T, 4, 4>& M, const Vector<T, 3>& v)
    {
        auto A = Matrix<T, 4, 4>::identity();
        A(0, 3) = v[0];
        A(1, 3) = v[1];
        A(2, 3) = v[2];
        return M * A;
    }

    template<typename T = float>
    Matrix<T, 4, 4> rotate_x(const Matrix<T, 4, 4>& M, float radians)
    {
        auto R = Matrix<T, 4, 4>::identity();
        const auto cos_t = std::cos(radians);
        const auto sin_t = std::sin(radians);
        R(1, 1) = cos_t;
        R(1, 2) = -sin_t;
        R(2, 1) = sin_t;
        R(2, 2) = cos_t;
        return M * R;
    }

    template<typename T = float>
    Matrix<T, 4, 4> rotate_y(const Matrix<T, 4, 4>& M, float radians)
    {
        auto R = Matrix<T, 4, 4>::identity();
        const auto cos_t = std::cos(radians);
        const auto sin_t = std::sin(radians);
        R(0, 0) = cos_t;
        R(0, 2) = sin_t;
        R(2, 0) = -sin_t;
        R(2, 2) = cos_t;
        return M * R;
    }

    template<typename T = float>
    Matrix<T, 4, 4> rotate_z(const Matrix<T, 4, 4>& M, float radians)
    {
        auto R = Matrix<T, 4, 4>::identity();
        const auto cos_t = std::cos(radians);
        const auto sin_t = std::sin(radians);
        R(0, 0) = cos_t;
        R(0, 1) = -sin_t;
        R(1, 0) = sin_t;
        R(1, 1) = cos_t;
        return M * R;
    }

    template<typename T = float>
    Matrix<T, 4, 4> scale(const Matrix<T, 4, 4>& M, const Vector<T, 3>& s)
    {
        auto S = Matrix<T, 4, 4>::identity();
        S(0, 0) = s[0];
        S(1, 1) = s[1];
        S(2, 2) = s[2];
        return M * S;
    }
} // namespace orion