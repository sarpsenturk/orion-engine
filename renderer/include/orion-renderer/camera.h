#pragma once

#include "orion-math/angles.h"
#include "orion-math/matrix/matrix4.h"
#include "orion-math/vector/vector3.h"

namespace orion
{
    class Camera
    {
    public:
        Camera() = default;
        Camera(Matrix4_f view, Matrix4_f projection);

        [[nodiscard]] const Matrix4_f& view() const { return view_; }
        [[nodiscard]] const Matrix4_f& projection() const { return projection_; }
        [[nodiscard]] const Matrix4_f& view_projection() const { return view_projection_; }

        void set_view(Matrix4_f view);
        void set_projection(Matrix4_f projection);
        void set_view_projection(Matrix4_f view, Matrix4_f projection);

    private:
        Matrix4_f calculate_view_projection() const;

        Matrix4_f view_ = Matrix4_f::identity();
        Matrix4_f projection_ = Matrix4_f::identity();
        Matrix4_f view_projection_ = calculate_view_projection();
    };

    // This is a temporary/basic solution to use before we can render scenes
    // we should be using the camera transformation
    class CameraController
    {
    public:
        explicit CameraController(Camera* camera, Radian_f fov, float aspect_ratio, Vector3_f position = {}, Radian_f pitch = radians(0.f), Radian_f yaw = degrees(-90.f));

        void translate(Vector3_f offset);
        void set_pitch(Radian_f pitch);
        void set_yaw(Radian_f yaw);

        [[nodiscard]] auto& forward() const { return forward_; }
        [[nodiscard]] auto& right() const { return right_; }
        [[nodiscard]] Radian_f pitch() const { return pitch_; }
        [[nodiscard]] Radian_f yaw() const { return yaw_; }

    private:
        Vector3_f calc_direction() const;
        Vector3_f calc_right() const;
        Matrix4_f calc_view() const;
        Matrix4_f calc_projection() const;

        Camera* camera_;
        Radian_f fov_;
        float aspect_ratio_;
        Vector3_f position_;
        Radian_f pitch_;
        Radian_f yaw_;
        Vector3_f forward_;
        Vector3_f right_;
    };
} // namespace orion
