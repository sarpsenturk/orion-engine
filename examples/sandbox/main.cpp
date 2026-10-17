#include <orion/orion.hpp>

#include <orion/renderer/renderer.hpp>

class SandboxApp : public orion::Application
{
public:
    explicit SandboxApp(orion::Engine& engine)
        : m_window(engine.window())
        , m_renderer(engine.renderer())
    {
    }

    void on_update(float dt) override
    {
    }

    void on_render() override
    {
        ImGui::ShowDemoWindow();

        m_renderer.render();
    }

private:
    orion::Window& m_window;
    orion::Renderer& m_renderer;
};

int main()
{
    auto engine = orion::Engine{};
    engine.run(std::make_unique<SandboxApp>(engine));
    return 0;
}
