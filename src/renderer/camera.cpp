#include "orion/renderer/camera.hpp"

#include "orion/math/matrix/projection.hpp"

namespace orion
{
    Camera::Camera(Matrix4f view, Matrix4f projection, Color clear_color)
        : view_(view)
        , projection_(projection)
        , view_projection_(view * projection)
        , clear_color_(clear_color)
    {
    }

    void Camera::set_view(Matrix4f view)
    {
        view_ = view;
        view_projection_ = view * projection_;
    }

    void Camera::set_projection(Matrix4f projection)
    {
        projection_ = projection;
        view_projection_ = view_ * projection;
    }

    void Camera::set_clear_color(Color clear_color)
    {
        clear_color_ = clear_color;
    }

    void Camera::ortho(float left, float right, float bottom, float top, float near, float far)
    {
        set_projection(orthographic_rh(left, right, bottom, top, near, far));
    }
} // namespace orion
