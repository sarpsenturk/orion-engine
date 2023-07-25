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
        , renderer_({.device_select_fn = orion::device_select_discrete, .render_size = window_size})
    {
        // Get device from renderer
        auto* device = renderer_.device();

        // Create swapchain
        {
            const auto desc = orion::SwapchainDesc{
                .image_count = swapchain_image_count,
                .image_format = swapchain_image_format,
                .image_size = window_.size(),
                .image_usage = swapchain_image_usage,
            };
            swapchain_ = device->create_swapchain(window_, desc);
        }

        // Handle window resize
        window_.on_resize_end().subscribe([device, this](const auto& resize) {
            // Recreate swapchain
            {
                const auto desc = orion::SwapchainDesc{
                    .image_count = swapchain_image_count,
                    .image_format = swapchain_image_format,
                    .image_size = resize.size,
                    .image_usage = swapchain_image_usage,
                };
                device->recreate(swapchain_, desc);
            }

            // Resize renderer images
            renderer_.resize_images(resize.size);
        });
    }

private:
    void on_user_update([[maybe_unused]] orion::frame_time dt) override
    {
        window_.poll_events();
    }

    void on_user_render() override
    {
        renderer_.begin_frame();

        renderer_.end_frame();

        renderer_.present(swapchain_);
    }

    [[nodiscard]] bool user_should_exit() const noexcept override
    {
        return window_.should_close();
    }

    static constexpr auto window_position = orion::WindowPosition{400, 200};
    static constexpr auto window_size = orion::WindowSize{800, 600};
    static constexpr auto swapchain_image_count = 2;
    static constexpr auto swapchain_image_format = orion::Format::B8G8R8A8_Srgb;
    static constexpr auto swapchain_image_usage =
        orion::ImageUsageFlags::disjunction({orion::ImageUsage::ColorAttachment, orion::ImageUsage::TransferDst});

    orion::Window window_;
    orion::Renderer renderer_;
    orion::SwapchainHandle swapchain_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
