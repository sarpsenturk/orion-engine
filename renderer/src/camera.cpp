#include "orion-renderer/camera.h"

namespace orion
{
    Camera::Camera(Matrix4_f view, Matrix4_f projection)
        : view_(view)
        , projection_(projection)
    {
    }

    void Camera::set_view(Matrix4_f view)
    {
        view_ = view;
        update_view_projection();
    }

    void Camera::set_projection(Matrix4_f projection)
    {
        projection_ = projection;
        update_view_projection();
    }

    void Camera::update_view_projection()
    {
        view_projection_ = view_ * projection_;
    }

    PerspectiveCamera::PerspectiveCamera(
        Radians fov,
        float aspect_ratio,
        float near,
        float far,
        Matrix4_f view)
        : fov_(fov)
        , aspect_ratio_(aspect_ratio)
        , near_(near)
        , far_(far)
        , camera_(view, perspective_projection())
    {
    }

    Matrix4_f PerspectiveCamera::perspective_projection() const
    {
        return perspective_fov_rh(fov_, aspect_ratio_, near_, far_);
    }

    OrthographicCamera::OrthographicCamera(
        float left,
        float right,
        float bottom,
        float top,
        float near,
        float far,
        Matrix4_f view)
        : left_(left)
        , right_(right)
        , bottom_(bottom)
        , top_(top)
        , near_(near)
        , far_(far)
        , camera_(view, orthographic_projection())
    {
    }

    Matrix4_f OrthographicCamera::orthographic_projection() const
    {
        return orthographic_rh(left_, right_, bottom_, top_, near_, far_);
    }

    // Extern template instantiations
    template LogicalCamera<OrthographicCamera>;
    template LogicalCamera<PerspectiveCamera>;
} // namespace orion
