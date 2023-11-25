#include <orion-engine/orion-engine.h>

#include <orion-renderer/render_window.h>
#include <orion-renderer/renderer.h>

#include <orion-scene/scene.h>

#include <orion-math/vector/vector3.h>

#include <fmt/chrono.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>

class SandboxApp final : public orion::Application
{
public:
    SandboxApp()
        : renderer_({.device_select_fn = orion::device_select_discrete, .render_size = window_size})
        , render_window_({.device = renderer_.device(), .name = "Orion Sandbox", .position = window_position, .size = window_size})
    {
        render_window_.window().on_close().subscribe([this](const auto&) { exit_application(); });

        // Create entity
        auto entity = scene_.create_entity();
        // Translate entity
        entity.transform().translate({5.f, 0.f, 0.f});
        // device->write_descriptor(descriptor_set, bindings);
    }

private:
    void on_user_update([[maybe_unused]] orion::frame_time delta_time) override
    {
        render_window_.window().poll_events();
    }

    void on_user_render() override
    {
        renderer_.begin();
        renderer_.draw_test_triangle();
        renderer_.end();
    }

    static constexpr auto window_position = orion::WindowPosition{400, 200};
    static constexpr auto window_size = orion::WindowSize{1280, 720};

    orion::Renderer renderer_;
    orion::RenderWindow render_window_;
    orion::Scene scene_;
    std::unique_ptr<orion::Swapchain> swapchain_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
