#include <orion-engine/orion-engine.h>

#include <orion-core/window.h>

#include <orion-renderer/renderer.h>

#include <orion-math/vector/vector3.h>

#include <fmt/chrono.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>

class SandboxApp final : public orion::Application
{
public:
    SandboxApp()
        : window_({.name = "Orion Sandbox", .position = window_position, .size = window_size})
        , renderer_({.device_select_fn = orion::device_select_discrete, .render_size = window_size})
    {
        window_.on_close().subscribe([this](const auto&) { exit_application(); });

        // Create swapchain
        swapchain_ = renderer_.device()->create_swapchain({
            .window = &window_,
            .image_size = window_size,
        });
    }

private:
    void on_user_update([[maybe_unused]] orion::frame_time delta_time) override
    {
        window_.poll_events();
    }

    void on_user_render() override
    {
        swapchain_->present();
    }

    static constexpr auto window_position = orion::WindowPosition{400, 200};
    static constexpr auto window_size = orion::WindowSize{1280, 720};

    orion::Window window_;
    orion::Renderer renderer_;
    std::unique_ptr<orion::Swapchain> swapchain_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
