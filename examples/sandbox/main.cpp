#include <orion/orion.hpp>

#include <orion/renderer/renderer.hpp>

#include <array>

#include <glm/glm.hpp>

static constexpr auto vertices = std::array{
    orion::Vertex{.position = glm::vec3(-0.5f, -0.5f, 0.0f)},
    orion::Vertex{.position = glm::vec3(0.5f, -0.5f, 0.0f)},
    orion::Vertex{.position = glm::vec3(0.5f, 0.5f, 0.0f)},
    orion::Vertex{.position = glm::vec3(-0.5f, 0.5f, 0.0f)},
};
static constexpr auto indices = std::array{0u, 1u, 2u, 0u, 2u, 3u};

class SandboxApp : public orion::Application
{
public:
    explicit SandboxApp(orion::Engine& engine)
        : m_renderer(engine.renderer())
        , m_mesh(m_renderer.create_mesh(vertices, indices))
    {
    }

    ~SandboxApp()
    {
        m_renderer.destroy_mesh(m_mesh);
    }

    void on_update(float dt) override {}
    void on_render() override {}

private:
    orion::Renderer& m_renderer;
    orion::Mesh m_mesh;
};

int main()
{
    auto engine = orion::Engine{};
    engine.run(std::make_unique<SandboxApp>(engine));
    return 0;
}
