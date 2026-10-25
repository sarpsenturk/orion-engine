#include <orion/orion.hpp>

#include <orion/core/window.hpp>
#include <orion/renderer/renderer.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/vec3.hpp>

#include <array>

static constexpr auto quad_vertices = std::array{
    glm::vec3(-0.5f, -0.5f, 0.0f), // Vertex 1: position (x, y, z)
    glm::vec3(0.5f, -0.5f, 0.0f),  // Vertex 2: position (x, y, z)
    glm::vec3(0.5f, 0.5f, 0.0f),   // Vertex 3: position (x, y, z)
    glm::vec3(-0.5f, 0.5f, 0.0f)   // Vertex 4: position (x, y, z)
};
static constexpr auto quad_indices = std::array{
    0u, 1u, 2u, // First triangle
    2u, 3u, 0u  // Second triangle
};

class Sandbox : public orion::Application
{
public:
    explicit Sandbox(orion::Engine& engine)
        : m_window(orion::WindowDesc{"Sandbox", 1'280, 720})
        , m_renderer(engine.renderer())
        , m_swapchain(m_renderer.create_swapchain(m_window, 2u, orion::gpu::Format::BGRA8_Unorm))
        , m_quad_mesh(m_renderer.create_mesh({quad_vertices, quad_indices, 4u}))
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
            .instances = {{
                orion::RenderInstance{m_quad_mesh, glm::translate(glm::mat4(1.0f), glm::vec3(-1.f, 0.0f, 0.0f))},
                orion::RenderInstance{m_quad_mesh, glm::translate(glm::mat4(1.0f), glm::vec3(1.f, 0.0f, 0.0f))},
            }},
        });
    }

    void on_event(const orion::Event&) override {}

    bool should_exit() const override { return m_window.should_close(); }

    orion::Window m_window;
    orion::Renderer& m_renderer;
    orion::gpu::Swapchain* m_swapchain;

    orion::Mesh m_quad_mesh;
};

int main()
{
    auto engine = orion::Engine{};
    engine.run(std::make_unique<Sandbox>(engine));
    return 0;
}
