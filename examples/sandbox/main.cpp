#include <orion/orion.hpp>

#include <orion/renderer/imgui_renderer.hpp>
#include <orion/renderer/renderer.hpp>

#include <imgui.h>

class SandboxApp : public orion::Application
{
public:
    explicit SandboxApp(orion::Engine& engine)
        : m_imgui_renderer(engine.imgui_renderer())
    {
    }

    void on_update(float dt) override
    {
    }

    void on_render(orion::RenderFrame& frame) override
    {
        ImGui::ShowDemoWindow();

        frame.backbuffer = m_imgui_renderer.add_pass(frame.graph, frame.backbuffer, orion::gpu::LoadOp::Clear);
    }

private:
    orion::ImGuiRenderer& m_imgui_renderer;
};

int main()
{
    auto engine = orion::Engine{};
    engine.run(std::make_unique<SandboxApp>(engine));
    return 0;
}
