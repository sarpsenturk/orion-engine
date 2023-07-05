#pragma once

#include <orion-math/matrix/matrix4.h>
#include <orion-math/matrix/transformation.h>
#include <orion-math/vector/vector3.h>

namespace orion
{
    class Camera
    {
    public:
        Camera() = default;
        Camera(Matrix4_f view, Matrix4_f projection);

        [[nodiscard]] auto& view() const noexcept { return view_; }
        [[nodiscard]] auto& projection() const noexcept { return projection_; }
        [[nodiscard]] auto& view_projection() const noexcept { return view_projection_; }

        void set_view(Matrix4_f view);
        void set_projection(Matrix4_f projection);

    private:
        void update_view_projection();

        Matrix4_f view_ = Matrix4_f::identity();
        Matrix4_f projection_ = Matrix4_f::identity();
        Matrix4_f view_projection_ = view_ * projection_;
    };

    class PerspectiveCamera
    {
    public:
        PerspectiveCamera(
            Radians fov,
            float aspect_ratio,
            float near,
            float far,
            Matrix4_f view = Matrix4_f::identity());

        [[nodiscard]] auto fov() const noexcept { return fov_; }
        [[nodiscard]] auto aspect_ratio() const noexcept { return aspect_ratio_; }
        [[nodiscard]] auto near() const noexcept { return near_; }
        [[nodiscard]] auto far() const noexcept { return far_; }
        [[nodiscard]] auto& camera() const noexcept { return camera_; }
        [[nodiscard]] auto& camera() noexcept { return camera_; }

    private:
        Matrix4_f perspective_projection() const;

        Radians fov_;
        float aspect_ratio_;
        float near_;
        float far_;

        Camera camera_;
    };

    class OrthographicCamera
    {
    public:
        OrthographicCamera(
            float left,
            float right,
            float bottom,
            float top,
            float near,
            float far,
            Matrix4_f view = Matrix4_f::identity());

        [[nodiscard]] auto& camera() noexcept { return camera_; }
        [[nodiscard]] auto& camera() const noexcept { return camera_; }

    private:
        Matrix4_f orthographic_projection() const;

        float left_;
        float right_;
        float bottom_;
        float top_;
        float near_;
        float far_;

        Camera camera_;
    };

    template<typename Camera>
    class LogicalCamera
    {
    public:
        using camera_type = Camera;

        template<typename... Args>
        LogicalCamera(Vector3_f position, Args&&... args)
            : position_(position)
            , camera_(std::forward<Args>(args)..., calculate_view())
        {
        }

        [[nodiscard]] auto& position() const noexcept { return position_; }
        [[nodiscard]] auto& camera() noexcept { return camera_; }
        [[nodiscard]] auto& camera() const noexcept { return camera_; }

        void set_position(Vector3_f position)
        {
            position_ = position;
            camera_.camera().set_view(calculate_view());
        }

    private:
        Matrix4_f calculate_view() const
        {
            return Matrix4_f::identity() * translation(-position_);
        }

        Vector3_f position_ = {0.f, 0.f, 0.f};

        camera_type camera_;
    };

    using SceneCameraOrtho = LogicalCamera<OrthographicCamera>;
    using SceneCameraPerspective = LogicalCamera<PerspectiveCamera>;

    extern template LogicalCamera<OrthographicCamera>;
    extern template LogicalCamera<PerspectiveCamera>;
} // namespace orion
