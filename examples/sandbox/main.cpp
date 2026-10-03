#include <orion/orion.hpp>

#include <orion/core/input.hpp>
#include <orion/core/window.hpp>
#include <orion/renderer/renderer.hpp>

#include <array>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

static constexpr auto quad_vertices = std::array{
    orion::Vertex{.position = glm::vec3(-0.5f, -0.5f, 0.0f), .texcoord = glm::vec2(0.0f, 0.0f)},
    orion::Vertex{.position = glm::vec3(0.5f, -0.5f, 0.0f), .texcoord = glm::vec2(1.0f, 0.0f)},
    orion::Vertex{.position = glm::vec3(0.5f, 0.5f, 0.0f), .texcoord = glm::vec2(1.0f, 1.0f)},
    orion::Vertex{.position = glm::vec3(-0.5f, 0.5f, 0.0f), .texcoord = glm::vec2(0.0f, 1.0f)},
};
static constexpr auto quad_indices      = std::array{0u, 1u, 2u, 0u, 2u, 3u};
static constexpr auto triangle_vertices = std::array{
    orion::Vertex{.position = glm::vec3(-0.5f, -0.5f, 0.0f), .texcoord = glm::vec2(0.0f, 0.0f)},
    orion::Vertex{.position = glm::vec3(0.5f, -0.5f, 0.0f), .texcoord = glm::vec2(1.0f, 0.0f)},
    orion::Vertex{.position = glm::vec3(0.0f, 0.5f, 0.0f), .texcoord = glm::vec2(0.5f, 1.0f)},
};
static constexpr auto triangle_indices = std::array{0u, 1u, 2u};

class SandboxApp : public orion::Application
{
public:
    explicit SandboxApp(orion::Engine& engine)
        : m_window(engine.window())
        , m_renderer(engine.renderer())
        , m_input(engine.input())
        , m_quad(m_renderer.create_mesh(quad_vertices, quad_indices))
        , m_triangle(m_renderer.create_mesh(triangle_vertices, triangle_indices))
        , m_green_texture(m_renderer.create_texture(
              orion::TextureDesc{
                  .width  = 1u,
                  .height = 1u,
                  .depth  = 1u,
              },
              std::array{std::byte{0x00}, std::byte{0xff}, std::byte{0x00}, std::byte{0xff}}))
        , m_green_material(m_renderer.create_material({.albedo = m_green_texture}))
        , m_red_texture(m_renderer.create_texture(
              orion::TextureDesc{
                  .width  = 1u,
                  .height = 1u,
                  .depth  = 1u,
              },
              std::array{std::byte{0xff}, std::byte{0x00}, std::byte{0x00}, std::byte{0xff}}))
        , m_red_material(m_renderer.create_material({.albedo = m_red_texture}))
    {
    }

    ~SandboxApp()
    {
        m_renderer.destroy_texture(m_red_texture);
        m_renderer.destroy_material(m_red_material);
        m_renderer.destroy_texture(m_green_texture);
        m_renderer.destroy_material(m_green_material);
        m_renderer.destroy_mesh(m_triangle);
        m_renderer.destroy_mesh(m_quad);
    }

    void on_update(float dt) override
    {
        // Update pitch and yaw
        if (m_input.is_key_pressed(orion::Keycode::Up)) {
            m_camera_pitch += m_camera_sensitivity * dt * 100.0f;
        }
        if (m_input.is_key_pressed(orion::Keycode::Down)) {
            m_camera_pitch -= m_camera_sensitivity * dt * 100.0f;
        }
        if (m_input.is_key_pressed(orion::Keycode::Left)) {
            m_camera_yaw -= m_camera_sensitivity * dt * 100.0f;
        }
        if (m_input.is_key_pressed(orion::Keycode::Right)) {
            m_camera_yaw += m_camera_sensitivity * dt * 100.0f;
        }

        // Clamp pitch & yaw
        m_camera_pitch = glm::clamp(m_camera_pitch, -89.9f, 89.9f);
        m_camera_yaw   = glm::mod(m_camera_yaw, 360.0f);

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
        if (m_input.is_key_pressed(orion::Keycode::W)) {
            m_camera_position += m_camera_forward * m_camera_speed * dt;
        }
        if (m_input.is_key_pressed(orion::Keycode::S)) {
            m_camera_position -= m_camera_forward * m_camera_speed * dt;
        }
        if (m_input.is_key_pressed(orion::Keycode::A)) {
            m_camera_position -= m_camera_right * m_camera_speed * dt;
        }
        if (m_input.is_key_pressed(orion::Keycode::D)) {
            m_camera_position += m_camera_right * m_camera_speed * dt;
        }
    }

    void on_render() override
    {
        // Draw imgui demo window
        ImGui::ShowDemoWindow();

        // Camera properties
        ImGui::Begin("Camera");
        ImGui::Text("Use arrow keys to rotate camera and WASD to move camera");
        ImGui::SliderFloat3("Position", &m_camera_position.x, -100.0f, 100.0f);
        ImGui::SliderFloat("Speed", &m_camera_speed, 0.1f, 10.0f);
        ImGui::SliderFloat("Sensitivity", &m_camera_sensitivity, 0.01f, 1.0f);
        ImGui::SliderFloat("Pitch", &m_camera_pitch, -89.9f, 89.9f);
        ImGui::SliderFloat("Yaw", &m_camera_yaw, 0.0f, 360.0f);
        ImGui::End();

        m_renderer.render({
            .camera = {
                .view       = glm::lookAtRH(m_camera_position, m_camera_position + m_camera_forward, m_camera_up),
                .projection = glm::perspectiveRH_ZO(glm::radians(60.0f), m_window.aspect_ratio(), 0.1f, 100.0f),
            },
            .objects = {{
                orion::RenderObject{
                    .mesh     = m_quad,
                    .model    = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f)),
                    .material = m_green_material,
                },
                orion::RenderObject{
                    .mesh     = m_triangle,
                    .model    = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f)),
                    .material = m_red_material,
                },
            }},
        });
    }

private:
    orion::Window& m_window;
    orion::Renderer& m_renderer;
    orion::Input& m_input;

    orion::Mesh m_quad;
    orion::Mesh m_triangle;
    orion::Texture m_green_texture;
    orion::Material m_green_material;
    orion::Texture m_red_texture;
    orion::Material m_red_material;

    glm::vec3 m_camera_position = glm::vec3(0.0f, 0.0f, 2.0f);
    float m_camera_speed        = 2.5f;
    float m_camera_pitch        = 0.0f;
    float m_camera_yaw          = 0.0f;
    float m_camera_sensitivity  = 0.5f;
    glm::vec3 m_camera_forward  = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 m_camera_up       = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 m_camera_right    = glm::vec3(1.0f, 0.0f, 0.0f);
};

int main()
{
    auto engine = orion::Engine{};
    engine.run(std::make_unique<SandboxApp>(engine));
    return 0;
}
