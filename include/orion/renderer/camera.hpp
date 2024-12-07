#pragma once

#include "orion/renderer/color.hpp"

#include "orion/math/matrix/matrix4.hpp"

namespace orion
{
    class Camera
    {
    public:
        Camera() = default;
        Camera(Matrix4f view, Matrix4f projection, Color clear_color);

        void set_view(Matrix4f view);
        void set_projection(Matrix4f projection);
        void set_clear_color(Color clear_color);

        [[nodiscard]] Matrix4f view() const { return view_; }
        [[nodiscard]] Matrix4f projection() const { return projection_; }
        [[nodiscard]] Matrix4f view_projection() const { return view_projection_; }
        [[nodiscard]] Color clear_color() const { return clear_color_; }

        void ortho(float left, float right, float bottom, float top, float near, float far);

    private:
        Matrix4f view_ = Matrix4f::identity();
        Matrix4f projection_ = Matrix4f::identity();
        Matrix4f view_projection_ = Matrix4f::identity();
        Color clear_color_ = colors::magenta;
    };
} // namespace orion
