#include <orion/orion.hpp>

#include <orion/core/window.hpp>
#include <orion/renderer/renderer.hpp>

class Sandbox : public orion::Application
{
public:
    explicit Sandbox(orion::Engine& engine)
        : m_window(orion::WindowDesc{"Sandbox", 1'280, 720})
        , m_renderer(engine.renderer())
        , m_swapchain(m_renderer.create_swapchain(m_window, 2u, orion::gpu::Format::BGRA8_Unorm))
    {
    }

private:
    void on_update() override {}

    void on_render() override
    {
        if (m_window.width() == 0 || m_window.height() == 0) { return; }
        auto frame = m_renderer.begin_frame(m_swapchain);
        if (!frame) { return; }
        m_renderer.end_frame();
    }

    void on_event(const orion::Event&) override {}

    bool should_exit() const override { return m_window.should_close(); }

    orion::Window m_window;
    orion::Renderer& m_renderer;
    orion::gpu::Swapchain* m_swapchain;
};

int main()
{
    auto engine = orion::Engine{};
    engine.run(std::make_unique<Sandbox>(engine));
    return 0;
}
