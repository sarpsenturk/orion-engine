#include <orion-engine/orion-engine.h>

#include <orion-core/window.h>

#include <orion-renderer/renderer.h>

#include <orion-math/vector/vector3.h>

#include <fmt/chrono.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <array>

class SandboxApp : public orion::Application
{
public:
    SandboxApp()
        : window_({.name = "Orion Sandbox", .position = window_position, .size = window_size})
        , renderer_({.device_select_fn = orion::device_select_discrete})
    {
        create_surface();
        create_swapchain();

        window_.on_resize_end() += ([this](const auto&) {
            create_swapchain();
        });
    }

private:
    void on_user_update([[maybe_unused]] orion::frame_time dt) override
    {
        window_.poll_events();
    }

    void on_user_render() override
    {
    }

    [[nodiscard]] bool user_should_exit() const noexcept override
    {
        return window_.should_close();
    }

    void create_surface()
    {
        surface_ = renderer_.device()->make_unique(orion::SurfaceHandle_tag{}, window_);
    }

    void create_swapchain()
    {
        const auto desc = orion::SwapchainDesc{
            .surface = surface_.get(),
            .image_count = swapchain_image_count,
            .image_format = swapchain_image_format,
            .image_size = window_.size(),
            .image_usage = swapchain_image_usage,
        };
        swapchain_ = renderer_.device()->make_unique(orion::SwapchainHandle_tag{}, desc);
    }

    static constexpr auto window_position = orion::WindowPosition{400, 200};
    static constexpr auto window_size = orion::WindowSize{800, 600};
    static constexpr auto swapchain_image_count = 2;
    static constexpr auto swapchain_image_format = orion::Format::B8G8R8A8_Srgb;
    static constexpr auto swapchain_image_usage =
        orion::ImageUsageFlags::disjunction({orion::ImageUsage::ColorAttachment, orion::ImageUsage::TransferDst});

    orion::Window window_;
    orion::Renderer renderer_;
    orion::UniqueSurface surface_;
    orion::UniqueSwapchain swapchain_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
