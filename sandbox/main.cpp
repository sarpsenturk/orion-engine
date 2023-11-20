#include <orion-engine/orion-engine.h>

#include <orion-core/window.h>

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
        : window_({.name = "Orion Sandbox", .position = window_position, .size = window_size})
        , renderer_({.device_select_fn = orion::device_select_discrete, .render_size = window_size})
    {
        window_.on_close().subscribe([this](const auto&) { exit_application(); });

        // Create swapchain
        swapchain_ = renderer_.device()->create_swapchain({
            .window = &window_,
            .image_size = window_size,
        });
        window_.on_resize_end().subscribe([this](const auto& resize) {
            swapchain_->resize_images(2, orion::Format::B8G8R8A8_Srgb, resize.size, orion::ImageUsageFlags::ColorAttachment);
        });

        // Create entity
        auto entity = scene_.create_entity();
        // Translate entity
        entity.transform().translate({5.f, 0.f, 0.f});

        // device->write_descriptor(descriptor_set, bindings);
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
        renderer_.end();
    }

    static constexpr auto window_position = orion::WindowPosition{400, 200};
    static constexpr auto window_size = orion::WindowSize{1280, 720};

    orion::Window window_;
    orion::Renderer renderer_;
    orion::Scene scene_;
    std::unique_ptr<orion::Swapchain> swapchain_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
