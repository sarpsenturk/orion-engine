#pragma once

#include "orion/core/input.hpp"
#include "orion/renderer/camera.hpp"

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace orion
{
    class FlyCamera
    {
    public:
        explicit FlyCamera(Input& input);
        void update(float dt);

        [[nodiscard]] float pitch() const { return m_camera_pitch; }
        [[nodiscard]] float yaw() const { return m_camera_yaw; }
        [[nodiscard]] const glm::vec3& position() const { return m_camera_position; }
        [[nodiscard]] const glm::vec3& forward() const { return m_camera_forward; }
        [[nodiscard]] const glm::vec3& up() const { return m_camera_up; }
        [[nodiscard]] const glm::vec3& right() const { return m_camera_right; }
        [[nodiscard]] glm::vec3 center() const { return m_camera_position + m_camera_forward; }

    private:
        Input* m_input;
        MousePosition m_last_mouse_pos;

        glm::vec3 m_camera_position = glm::vec3(0.0f, 0.0f, 2.0f);
        float m_camera_speed        = 2.5f;
        float m_camera_pitch        = 0.0f;
        float m_camera_yaw          = 0.0f;
        float m_camera_sensitivity  = 0.5f;
        glm::vec3 m_camera_forward  = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 m_camera_up       = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 m_camera_right    = glm::vec3(1.0f, 0.0f, 0.0f);
        int m_invert_y_axis         = -1;
    };
} // namespace orion
