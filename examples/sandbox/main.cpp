#include <orion/orion.hpp>

#include <orion/core/window.hpp>
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
    0u, 1u, 2u, 2u, 3u, 0u, // Back face
    4u, 5u, 6u, 6u, 7u, 4u, // Front face
    0u, 4u, 7u, 7u, 3u, 0u, // Left face
    1u, 5u, 6u, 6u, 2u, 1u, // Right face
    3u, 2u, 6u, 6u, 7u, 3u, // Top face
    0u, 1u, 5u, 5u, 4u, 0u  // Bottom face
};

class Sandbox : public orion::Application
{
public:
    explicit Sandbox(orion::Engine& engine)
        : m_window(orion::WindowDesc{"Sandbox", 1'280, 720})
        , m_renderer(engine.renderer())
        , m_swapchain(m_renderer.create_swapchain(m_window, 2u, orion::gpu::Format::BGRA8_Unorm))
        , m_cube_mesh(m_renderer.create_mesh({cube_vertices, cube_indices, 8u}))
    {
    }

private:
    void on_update() override {}

    void on_render() override
    {
        if (m_window.width() == 0 || m_window.height() == 0) { return; }
        auto frame = m_renderer.begin_frame(m_swapchain);
        if (!frame) { return; }
        m_renderer.render({
            .view       = glm::lookAtRH(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
            .projection = glm::perspectiveRH_ZO(glm::radians(45.0f), m_window.aspect_ratio(), 0.1f, 100.0f),
            .scene      = {
                .instances = {{
                    orion::RenderInstance{m_cube_mesh, glm::translate(glm::mat4(1.0f), glm::vec3(-1.f, 0.0f, 0.0f))},
                    orion::RenderInstance{m_cube_mesh, glm::translate(glm::mat4(1.0f), glm::vec3(1.f, 0.0f, 0.0f))},
                }},
            },
        });
    }

    void on_event(const orion::Event&) override {}

    bool should_exit() const override { return m_window.should_close(); }

    orion::Window m_window;
    orion::Renderer& m_renderer;
    orion::gpu::Swapchain* m_swapchain;

    orion::Mesh m_cube_mesh;
};

int main()
{
    auto engine = orion::Engine{};
    engine.run(std::make_unique<Sandbox>(engine));
    return 0;
}
