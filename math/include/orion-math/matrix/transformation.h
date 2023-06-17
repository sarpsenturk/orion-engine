#pragma once

#include "matrix4.h"
#include "orion-math/angles.h"
#include "orion-math/trig.h"
#include "orion-math/vector/vector3.h"

namespace orion::math
{
    template<typename T>
    [[nodiscard]] constexpr Vector3_t<T> transform(const Vector3_t<T>& vector, const Matrix4_t<T>& transform)
    {
        const Matrix<T, 1, 4> vector_matrix{vector[0], vector[1], vector[2], T{1}};
        const auto result = vector_matrix * transform;
        return {result[0][0], result[0][1], result[0][2]};
    }

    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> scaling(T x, T y, T z)
    {
        return {
            x, 0, 0, 0,
            0, y, 0, 0,
            0, 0, z, 0,
            0, 0, 0, 1};
    }

    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> scaling(const Vector3_t<T>& vector)
    {
        return scaling(vector.x(), vector.y(), vector.z());
    }

    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> translation(T x, T y, T z)
    {
        return {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            x, y, z, 1};
    }

    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> translation(const Vector3_t<T>& vector)
    {
        return translation(vector.x(), vector.y(), vector.z());
    }

    template<typename T = float>
    [[nodiscard]] constexpr Matrix4_t<T> rotation_x(Radians radians)
    {
        const auto cos_x = cos<T>(radians);
        const auto sin_x = sin<T>(radians);
        return {
            1, 0, 0, 0,
            0, cos_x, sin_x, 0,
            0, -sin_x, cos_x, 0,
            0, 0, 0, 1};
    }

    template<typename T = float>
    [[nodiscard]] constexpr Matrix4_t<T> rotation_y(Radians radians)
    {
        const auto cos_x = cos<T>(radians);
        const auto sin_x = sin<T>(radians);
        return {
            cos_x, 0, -sin_x, 0,
            0, 1, 0, 0,
            sin_x, 0, cos_x, 0,
            0, 0, 0, 1};
    }

    template<typename T = float>
    [[nodiscard]] constexpr Matrix4_t<T> rotation_z(Radians radians)
    {
        const auto cos_x = cos<T>(radians);
        const auto sin_x = sin<T>(radians);
        return {
            cos_x, sin_x, 0, 0,
            -sin_x, cos_x, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1};
    }

    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> lookat_rh(const Vector3_t<T>& eye, const Vector3_t<T>& target, const Vector3_t<T>& up)
    {
        const auto zaxis = (eye - target).normalize();
        const auto xaxis = cross(up, zaxis).normalize();
        const auto yaxis = cross(zaxis, xaxis);
        return {
            xaxis.x(), yaxis.x(), zaxis.x(), 0,
            xaxis.y(), yaxis.y(), zaxis.y(), 0,
            xaxis.z(), yaxis.z(), zaxis.z(), 0,
            -dot(xaxis, eye), -dot(yaxis, eye), -dot(zaxis, eye), 1};
    }

    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> lookat_lh(const Vector3_t<T>& eye, const Vector3_t<T>& target, const Vector3_t<T>& up)
    {
        const auto zaxis = (target - eye).normalize();
        const auto xaxis = cross(up, zaxis).normalize();
        const auto yaxis = cross(zaxis, xaxis);
        return {
            xaxis.x(), yaxis.x(), zaxis.x(), 0,
            xaxis.y(), yaxis.y(), zaxis.y(), 0,
            xaxis.z(), yaxis.z(), zaxis.z(), 0,
            -dot(xaxis, eye), -dot(yaxis, eye), -dot(zaxis, eye), 1};
    }

    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> orthographic_rh(T left, T right, T bottom, T top, T near, T far)
    {
        const auto width = right - left;
        const auto height = top - bottom;
        const auto depth = near - far;
        return {
            T{2} / width, 0, 0, 0,
            0, T{2} / height, 0, 0,
            0, 0, T{1} / depth, 0,
            (left + right) / width, (top + bottom) / height, near / depth, 1};
    }

    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> orthographic_lh(T left, T right, T bottom, T top, T near, T far)
    {
        const auto width = right - left;
        const auto height = top - bottom;
        const auto depth = far - near;
        return {
            T{2} / width, 0, 0, 0,
            0, T{2} / height, 0, 0,
            0, 0, 1 / depth, 0,
            (left + right) / width, (top + bottom) / height, near / (near - far), 1};
    }

    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> perspective_fov_rh(Radians fov, T aspect_ratio, T near, T far)
    {
        const auto yscale = static_cast<T>(1 / tan<T>(fov / 2));
        const auto xscale = yscale / aspect_ratio;
        const auto zdiff = near - far;
        return {
            xscale, 0, 0, 0,
            0, yscale, 0, 0,
            0, 0, far / zdiff, T{-1},
            0, 0, (near * far) / zdiff, 0};
    }

    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> perspective_fov_lh(Radians fov, T aspect_ratio, T near, T far)
    {
        const auto yscale = static_cast<T>(1 / tan(fov / 2));
        const auto xscale = yscale / aspect_ratio;
        const auto zdiff = near - far;
        return {
            xscale, 0, 0, 0,
            0, yscale, 0, 0,
            0, 0, far / zdiff, T{1},
            0, 0, (-near * far) / zdiff, 0};
    }
} // namespace orion::math
