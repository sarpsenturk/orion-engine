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

        // Create render pass
        {
            const auto color_attachments = std::array{
                orion::RenderPassAttachmentDesc{
                    .format = swapchain_image_format,
                    .load_op = orion::AttachmentLoadOp::Clear,
                    .store_op = orion::AttachmentStoreOp::Store,
                    .initial_layout = orion::ImageLayout::Undefined,
                    .layout = orion::ImageLayout::ColorAttachment,
                    .final_layout = orion::ImageLayout::PresentSrc,
                },
            };
            const auto desc = orion::RenderPassDesc{
                .color_attachments = color_attachments,
            };
            render_pass_ = device->create_render_pass(desc);
        }

        // Create swapchain framebuffers
        {
            std::ranges::transform(swapchain_attachments_, swapchain_framebuffers_.begin(), [device, this](auto handle) {
                const auto desc = orion::FramebufferDesc{
                    .render_pass = render_pass_,
                    .attachments = {&handle, 1},
                    .size = window_.size(),
                };
                return device->create_framebuffer(desc);
            });
        }

        // Create swapchain image semaphores
        {
            std::generate(image_semaphores_.begin(), image_semaphores_.end(), [device]() {
                return device->create_semaphore();
            });
        }

        // Create render semaphore
        {
            render_semaphore_ = device->create_semaphore();
        }
    }

private:
    void on_user_update(orion::frame_time dt) override
    {
        window_.poll_events();
    }

    void on_user_render() override
    {
        // Get render device
        auto* device = renderer_.device();

        // Get current frame index
        const auto frame_index = renderer_.frame_index();

        // Get swapchain image semaphore for current frame
        const auto swapchain_image_semaphore = image_semaphores_[frame_index];

        // Get next swapchain image index and framebuffer
        const auto image_index = device->acquire_next_image(swapchain_, swapchain_image_semaphore, {});
        const auto framebuffer = swapchain_framebuffers_[image_index];

        // Begin frame
        renderer_.begin_frame({
            .render_pass = render_pass_,
            .framebuffer = framebuffer,
            .render_area = window_.size(),
            .clear_color = {1.f, 0.f, 1.f, 1.f},
        });

        // End frame
        const auto wait_semaphores = std::array{swapchain_image_semaphore};
        const auto signal_semaphores = std::array{render_semaphore_};
        renderer_.end_frame({
            .wait_semaphores = wait_semaphores,
            .signal_semaphores = signal_semaphores,
        });

        renderer_.present({.swapchain = swapchain_, .wait_semaphore = render_semaphore_, .image_index = image_index});
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
    orion::RenderPassHandle render_pass_;
    std::array<orion::FramebufferHandle, swapchain_image_count> swapchain_framebuffers_;
    std::array<orion::SemaphoreHandle, swapchain_image_count> image_semaphores_;
    orion::SemaphoreHandle render_semaphore_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
