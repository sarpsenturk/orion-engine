#include <orion/orion.hpp>

#include <orion/core/window.hpp>
#include <orion/renderer/renderer.hpp>

#include <array>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

static constexpr auto quad_vertices = std::array{
    orion::Vertex{.position = glm::vec3(-0.5f, -0.5f, 0.0f)},
    orion::Vertex{.position = glm::vec3(0.5f, -0.5f, 0.0f)},
    orion::Vertex{.position = glm::vec3(0.5f, 0.5f, 0.0f)},
    orion::Vertex{.position = glm::vec3(-0.5f, 0.5f, 0.0f)},
};
static constexpr auto quad_indices      = std::array{0u, 1u, 2u, 0u, 2u, 3u};
static constexpr auto triangle_vertices = std::array{
    orion::Vertex{.position = glm::vec3(-0.5f, -0.5f, 0.0f)},
    orion::Vertex{.position = glm::vec3(0.5f, -0.5f, 0.0f)},
    orion::Vertex{.position = glm::vec3(0.0f, 0.5f, 0.0f)},
};
static constexpr auto triangle_indices = std::array{0u, 1u, 2u};

class SandboxApp : public orion::Application
{
public:
    explicit SandboxApp(orion::Engine& engine)
        : m_window(engine.window())
        , m_renderer(engine.renderer())
        , m_quad(m_renderer.create_mesh(quad_vertices, quad_indices))
        , m_triangle(m_renderer.create_mesh(triangle_vertices, triangle_indices))
    {
    }

    ~SandboxApp()
    {
        m_renderer.destroy_mesh(m_triangle);
        m_renderer.destroy_mesh(m_quad);
    }

    void on_update(float dt) override {}

    void on_render() override
    {
        // Draw imgui demo window
        ImGui::ShowDemoWindow();

        m_renderer.render({
            .camera = {
                .view       = glm::lookAtRH(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
                .projection = glm::perspectiveRH_ZO(glm::radians(60.0f), m_window.aspect_ratio(), 0.1f, 100.0f),
            },
            .objects = {{
                orion::RenderObject{
                    .mesh  = m_quad,
                    .model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f)),
                },
                orion::RenderObject{
                    .mesh  = m_triangle,
                    .model = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, 0.0f, 0.0f)),
                },
            }},
        });
    }

private:
    orion::Window& m_window;
    orion::Renderer& m_renderer;
    orion::Mesh m_quad;
    orion::Mesh m_triangle;
};

int main()
{
    auto engine = orion::Engine{};
    engine.run(std::make_unique<SandboxApp>(engine));
    return 0;
}
