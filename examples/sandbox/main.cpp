#include <orion/orion.hpp>

#include <orion/core/window.hpp>
#include <orion/imgui/imgui.hpp>
#include <orion/renderer/renderer.hpp>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include <array>

static constexpr auto cube_vertices = std::array{
    glm::vec3(-0.5f, -0.5f, -0.5f), // Vertex 1: position (x, y, z)
    glm::vec3(0.5f, -0.5f, -0.5f),  // Vertex 2: position (x, y, z)
    glm::vec3(0.5f, 0.5f, -0.5f),   // Vertex 3: position (x, y, z)
    glm::vec3(-0.5f, 0.5f, -0.5f),  // Vertex 4: position (x, y, z)
    glm::vec3(-0.5f, -0.5f, 0.5f),  // Vertex 5: position (x, y, z)
    glm::vec3(0.5f, -0.5f, 0.5f),   // Vertex 6: position (x, y, z)
    glm::vec3(0.5f, 0.5f, 0.5f),    // Vertex 7: position (x, y, z)
    glm::vec3(-0.5f, 0.5f, 0.5f)    // Vertex 8: position (x, y, z)
};
static constexpr auto cube_indices = std::array{
    // Counterclockwise winding when viewed from outside the cube.
    0u, 2u, 1u, 2u, 0u, 3u, // Back face
    4u, 5u, 6u, 6u, 7u, 4u, // Front face
    0u, 4u, 7u, 7u, 3u, 0u, // Left face
    1u, 6u, 5u, 6u, 1u, 2u, // Right face
    3u, 6u, 2u, 6u, 3u, 7u, // Top face
    0u, 1u, 5u, 5u, 4u, 0u  // Bottom face
};

class Sandbox : public orion::Application
{
public:
    explicit Sandbox(orion::Engine& engine)
        : m_window(orion::WindowDesc{"Sandbox", 1'280, 720})
        , m_renderer(engine.renderer())
        , m_swapchain(m_renderer.create_swapchain(m_window))
        , m_imgui(m_renderer, m_window)
        , m_cube_mesh(m_renderer.create_mesh({cube_vertices, cube_indices, 8u}))
        , m_red_material(m_renderer.create_material({glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)}))
        , m_green_material(m_renderer.create_material({glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)}))
    {
        m_window.set_cursor_mode(orion::CursorMode::Disabled);
        m_last_mouse_position = m_window.mouse_position();
    }

private:
    void on_update(float delta_time) override
    {
        // Handle camera movement based on keyboard input
        float xdelta = 0.0f;
        if (m_window.is_key_pressed(orion::Keycode::A)) { xdelta -= 1.0f; }
        if (m_window.is_key_pressed(orion::Keycode::D)) { xdelta += 1.0f; }
        float ydelta = 0.0f;
        if (m_window.is_key_pressed(orion::Keycode::W)) { ydelta = 1.0f; }
        if (m_window.is_key_pressed(orion::Keycode::S)) { ydelta -= 1.0f; }

        // Handle camera rotation based on mouse movement
        if (m_cursor_locked) {
            const auto mouse_position = m_window.mouse_position();
            const auto mouse_delta    = mouse_position - m_last_mouse_position;
            m_last_mouse_position     = mouse_position;
            m_camera_pitch -= mouse_delta.y * m_camera_sensitivity;
            m_camera_yaw += mouse_delta.x * m_camera_sensitivity;
        }

        // Clamp pitch & wrap yaw
        m_camera_pitch = glm::clamp(m_camera_pitch, -89.0f, 89.0f);
        m_camera_yaw   = glm::mod(m_camera_yaw, 360.0f);

        // Calculate camera front & right vectors based on yaw and pitch
        m_camera_front.x = cos(glm::radians(m_camera_yaw)) * cos(glm::radians(m_camera_pitch));
        m_camera_front.y = sin(glm::radians(m_camera_pitch));
        m_camera_front.z = sin(glm::radians(m_camera_yaw)) * cos(glm::radians(m_camera_pitch));
        m_camera_front   = glm::normalize(m_camera_front);
        m_camera_right   = glm::normalize(glm::cross(m_camera_front, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_camera_up      = glm::normalize(glm::cross(m_camera_right, m_camera_front));

        // Update camera position based on input
        m_camera_position += m_camera_right * xdelta * m_camera_speed * delta_time;
        m_camera_position += m_camera_front * ydelta * m_camera_speed * delta_time;
    }

    void on_event(const orion::Event& event) override
    {
        if (const auto keypress = std::get_if<orion::OnKeyPress>(&event)) {
            if (m_imgui.wants_capture_keyboard()) { return; }
            if (keypress->key == orion::Keycode::Escape) {
                m_window.set_cursor_mode(orion::CursorMode::Normal);
                m_cursor_locked = false;
            }
        }
        if (const auto mouse_click = std::get_if<orion::OnMouseButtonPress>(&event)) {
            if (m_imgui.wants_capture_mouse()) { return; }
            if (mouse_click->button == orion::MouseButton::Left && !m_cursor_locked) {
                m_window.set_cursor_mode(orion::CursorMode::Disabled);
                m_last_mouse_position = m_window.mouse_position();
                m_cursor_locked       = true;
            }
        }
    }

    void on_render() override
    {
        m_imgui.begin_frame();
        ImGui::Begin("Camera Properties");
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", m_camera_position.x, m_camera_position.y, m_camera_position.z);
        ImGui::Text("Front: (%.2f, %.2f, %.2f)", m_camera_front.x, m_camera_front.y, m_camera_front.z);
        ImGui::Text("Up: (%.2f, %.2f, %.2f)", m_camera_up.x, m_camera_up.y, m_camera_up.z);
        ImGui::Text("Right: (%.2f, %.2f, %.2f)", m_camera_right.x, m_camera_right.y, m_camera_right.z);
        ImGui::SliderFloat("Pitch", &m_camera_pitch, -89.0f, 89.0f);
        ImGui::SliderFloat("Yaw", &m_camera_yaw, -180.0f, 180.0f);
        ImGui::SliderFloat("Speed", &m_camera_speed, 0.1f, 10.0f);
        ImGui::SliderFloat("Sensitivity", &m_camera_sensitivity, 0.01f, 1.0f);
        ImGui::End();
        m_imgui.end_frame();

        if (m_window.width() == 0 || m_window.height() == 0) { return; }
        if (auto frame = m_renderer.begin_frame(m_swapchain)) {
            m_renderer.render_scene({
                .view       = glm::lookAtRH(m_camera_position, m_camera_position + m_camera_front, m_camera_up),
                .projection = glm::perspectiveRH_ZO(glm::radians(45.0f), m_window.aspect_ratio(), 0.1f, 100.0f),
                .scene      = {
                    .instances = {{
                        orion::RenderInstance{m_cube_mesh, m_red_material, glm::translate(glm::mat4(1.0f), glm::vec3(-1.f, 0.0f, 0.0f))},
                        orion::RenderInstance{m_cube_mesh, m_green_material, glm::translate(glm::mat4(1.0f), glm::vec3(1.f, 0.0f, 0.0f))},
                    }},
                },
            });
            m_renderer.render_ui(ImGui::GetDrawData());
            m_renderer.end_frame();
        }

        m_imgui.update_platform_windows();
    }

    bool should_exit() const override { return m_window.should_close(); }

    orion::Window m_window;
    orion::Renderer& m_renderer;
    orion::gpu::Swapchain* m_swapchain;
    orion::ImGuiContext m_imgui;

    orion::Mesh m_cube_mesh;
    orion::Material m_red_material;
    orion::Material m_green_material;

    orion::MousePosition m_last_mouse_position{0.0f, 0.0f};
    glm::vec3 m_camera_position{0.0f, 0.0f, -5.0f};
    glm::vec3 m_camera_front{0.0f, 0.0f, 1.0f};
    glm::vec3 m_camera_up{0.0f, 1.0f, 0.0f};
    glm::vec3 m_camera_right{-1.0f, 0.0f, 0.0f};
    float m_camera_pitch{0.0f};
    float m_camera_yaw{90.0f};
    bool m_cursor_locked{true};
    float m_camera_sensitivity{0.1f};
    float m_camera_speed{5.0f};
};

int main()
{
    auto engine = orion::Engine{};
    engine.run(std::make_unique<Sandbox>(engine));
    return 0;
}
