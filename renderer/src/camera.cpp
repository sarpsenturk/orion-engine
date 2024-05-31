#include "orion-renderer/camera.h"

#include "orion-math/matrix/projection.h"
#include "orion-math/matrix/transformation.h"
#include "orion-math/trig.h"

namespace orion
{
    Camera::Camera(Matrix4_f view, Matrix4_f projection)
        : view_(view)
        , projection_(projection)
    {
    }

    Matrix4_f Camera::calculate_view_projection() const
    {
        return view_ * projection_;
    }

    void Camera::set_view(Matrix4_f view)
    {
        view_ = view;
        view_projection_ = calculate_view_projection();
    }

    void Camera::set_projection(Matrix4_f projection)
    {
        projection_ = projection;
        view_projection_ = calculate_view_projection();
    }

    void Camera::set_view_projection(Matrix4_f view, Matrix4_f projection)
    {
        view_ = view;
        projection_ = projection;
        view_projection_ = calculate_view_projection();
    }

    CameraController::CameraController(Camera* camera, Radian_f fov, float aspect_ratio, Vector3_f position, Radian_f pitch, Radian_f yaw)
        : camera_(camera)
        , fov_(fov)
        , aspect_ratio_(aspect_ratio)
        , position_(position)
        , pitch_(pitch)
        , yaw_(yaw)
        , forward_(calc_direction())
        , right_(calc_right())
    {
        camera_->set_view_projection(calc_view(), calc_projection());
    }

    Vector3_f CameraController::calc_direction() const
    {
        return vec3(cos(yaw_) * cos(pitch_),
                    sin(pitch_),
                    sin(yaw_) * cos(pitch_));
    }

    Vector3_f CameraController::calc_right() const
    {
        return cross(forward_, vec3(0.f, 1.f, 0.f)).normalize();
    }

    Matrix4_f CameraController::calc_view() const
    {
        return lookat_rh(position_, position_ + forward_, vec3(0.f, 1.f, 0.f));
    }

    Matrix4_f CameraController::calc_projection() const
    {
        return perspective_fov_rh(fov_, aspect_ratio_, .1f, 100.f);
    }

    void CameraController::translate(Vector3_f offset)
    {
        position_ += offset;
        camera_->set_view(calc_view());
    }

    void CameraController::set_pitch(Radian_f pitch)
    {
        pitch_ = pitch;
        forward_ = calc_direction();
        right_ = calc_right();
        camera_->set_view(calc_view());
    }

    void CameraController::set_yaw(Radian_f yaw)
    {
        yaw_ = yaw;
        forward_ = calc_direction();
        right_ = calc_right();
        camera_->set_view(calc_view());
    }
} // namespace orion
