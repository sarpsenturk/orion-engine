#include <fmt/chrono.h>
#include <orion-core/window.h>
#include <orion-engine/orion-engine.h>
#include <orion-math/vector/vector3.h>
#include <orion-renderer/renderer.h>
#include <spdlog/spdlog.h>

class SandboxApp : public orion::Application
{
public:
    SandboxApp()
        : window_({.name = "Orion Sandbox", .position = window_position, .size = window_size})
        , renderer_({.device_select_fn = orion::select_discrete})
    {
        // Get device from renderer
        auto* device = renderer_.device();

        // Create swapchain
        {
            const auto desc = orion::SwapchainDesc{
                .image_count = swapchain_image_count,
                .image_format = swapchain_image_format,
                .image_size = window_.size(),
            };
            swapchain_ = device->create_swapchain(window_, desc);
        }

        // Create swapchain attachments
        {
            const auto desc = orion::SwapchainAttachmentDesc{
                .swapchain = swapchain_,
                .format = swapchain_image_format,
            };
            device->create_swapchain_attachments(desc, swapchain_attachments_);
        }
    }

private:
    void on_user_update(orion::frame_time dt) override
    {
        window_.poll_events();
    }

    void on_user_render() override
    {
        renderer_.begin_frame();

        renderer_.end_frame();
        renderer_.present();
    }

    [[nodiscard]] bool user_should_exit() const noexcept override
    {
        return window_.should_close();
    }

    static constexpr auto window_position = orion::WindowPosition{400, 200};
    static constexpr auto window_size = orion::WindowSize{800, 600};
    static constexpr auto swapchain_image_count = 2;
    static constexpr auto swapchain_image_format = orion::Format::B8G8R8A8_Srgb;

    orion::Window window_;
    orion::Renderer renderer_;
    orion::SwapchainHandle swapchain_;
    std::array<orion::AttachmentHandle, swapchain_image_count> swapchain_attachments_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
