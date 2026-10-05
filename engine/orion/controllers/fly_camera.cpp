#include "fly_camera.hpp"

#include <glm/glm.hpp>

namespace orion
{
    FlyCamera::FlyCamera(Input& input)
        : m_input(&input)
        , m_last_mouse_pos(m_input->mouse_position())
    {
    }

    void FlyCamera::update(float dt)
    {
        // Update pitch and yaw based on mouse movement
        const auto current_mouse_pos = m_input->mouse_position();
        const auto mouse_delta_x     = static_cast<float>(current_mouse_pos.x - m_last_mouse_pos.x);
        const auto mouse_delta_y     = static_cast<float>(current_mouse_pos.y - m_last_mouse_pos.y);
        m_camera_pitch += mouse_delta_y * m_camera_sensitivity * m_invert_y_axis;
        m_camera_yaw += mouse_delta_x * m_camera_sensitivity;
        m_last_mouse_pos = current_mouse_pos;
        m_camera_pitch   = glm::clamp(m_camera_pitch, -89.9f, 89.9f);
        m_camera_yaw     = glm::mod(m_camera_yaw, 360.0f);

        // Calculate camera forward vector
        const auto pitch_rad = glm::radians(m_camera_pitch);
        const auto yaw_rad   = glm::radians(m_camera_yaw);
        m_camera_forward.x   = cos(pitch_rad) * sin(yaw_rad);
        m_camera_forward.y   = sin(pitch_rad);
        m_camera_forward.z   = -cos(pitch_rad) * cos(yaw_rad);
        m_camera_forward     = glm::normalize(m_camera_forward);

        // Calculate camera up & right vectors
        const auto world_up = glm::vec3(0.0f, 1.0f, 0.0f);
        m_camera_right      = glm::normalize(glm::cross(m_camera_forward, world_up));
        m_camera_up         = glm::normalize(glm::cross(m_camera_right, m_camera_forward));

        // Update camera position
        if (m_input->is_key_pressed(orion::Keycode::W)) {
            m_camera_position += m_camera_forward * m_camera_speed * dt;
        }
        if (m_input->is_key_pressed(orion::Keycode::S)) {
            m_camera_position -= m_camera_forward * m_camera_speed * dt;
        }
        if (m_input->is_key_pressed(orion::Keycode::A)) {
            m_camera_position -= m_camera_right * m_camera_speed * dt;
        }
        if (m_input->is_key_pressed(orion::Keycode::D)) {
            m_camera_position += m_camera_right * m_camera_speed * dt;
        }
    }
} // namespace orion
