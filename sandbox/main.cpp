#include <orion-engine/orion-engine.h>

#include <orion-core/window.h>

#include <orion-renderer/renderer.h>

#include <orion-scene/scene.h>

#include <orion-math/vector/vector3.h>

#include <fmt/chrono.h>
#include <spdlog/spdlog.h>

#include <imgui.h>

#include <algorithm>
#include <array>

class SandboxApp final : public orion::Application
{
public:
    SandboxApp()
        : renderer_({.device_select_fn = orion::device_select_discrete, .render_size = {800, 600}})
        , window_({})
        , render_window_(renderer_.create_render_window(window_))
    {
        // Close app on callback
        window_.on_close().subscribe(ORION_EXIT_APP_FN);

        // Initialize imgui
        renderer_.imgui_init(render_window_);

        // Create entity
        auto entity = scene_.create_entity();
        // Translate entity
        entity.transform().translate({5.f, 0.f, 0.f});
    }

private:
    void on_user_update([[maybe_unused]] orion::frame_time delta_time) override
    {
        window_.poll_events();
    }

    void on_user_render() override
    {
        renderer_.begin();

        renderer_.draw_test_triangle();

        renderer_.imgui_begin();
        ImGui::ShowDemoWindow();
        renderer_.imgui_end();

        renderer_.end();
        renderer_.present(render_window_);
    }

    orion::Renderer renderer_;
    orion::Window window_;
    orion::RenderWindow render_window_;
    orion::Scene scene_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
