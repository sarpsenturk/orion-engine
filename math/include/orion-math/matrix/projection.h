#pragma once

#include "matrix4.h"

#include "orion-math/angles.h"

namespace orion
{
    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> orthographic_rh(T left, T right, T bottom, T top, T near, T far)
    {
        auto projection = Matrix4_t<T>::identity();
        projection(0, 0) = T{2} / (right - left);
        projection(1, 1) = T{2} / (top - bottom);
        projection(2, 2) = -T{2} / (far - near);
        projection(3, 0) = -(right + left) / (right - left);
        projection(3, 1) = -(top + bottom) / (top - bottom);
        projection(3, 2) = -(far + near) / (far - near);
        return projection;
    }

    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> orthographic_lh(T left, T right, T bottom, T top, T near, T far)
    {
        auto projection = Matrix4_t<T>::identity();
        projection(0, 0) = T{2} / (right - left);
        projection(1, 1) = T{2} / (top - bottom);
        projection(2, 2) = T{2} / (far - near);
        projection(3, 0) = -(right + left) / (right - left);
        projection(3, 1) = -(top + bottom) / (top - bottom);
        projection(3, 2) = -(far + near) / (far - near);
        return projection;
    }

    template<typename T>
    [[nodiscard]] constexpr Matrix4_t<T> perspective_fov_rh(Radian_t<T> fov, T aspect_ratio, T near, T far)
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
    [[nodiscard]] constexpr Matrix4_t<T> perspective_fov_lh(Radian_t<T> fov, T aspect_ratio, T near, T far)
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
} // namespace orion
