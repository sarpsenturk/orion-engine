#include "orion-renderer/render_window.h"

#include "orion-renderapi/render_device.h"

#include "orion-utils/assertion.h"

#include <array>

namespace orion
{
    namespace
    {
        WindowDesc get_window_desc(const RenderWindowDesc& desc)
        {
            return {.name = desc.name, .position = desc.position, .size = desc.size};
        }

        SwapchainDesc get_swapchain_desc(const RenderWindowDesc& desc)
        {
            return {.image_count = desc.image_count, .image_format = desc.image_format, .image_size = desc.size, .image_usage = desc.image_usage};
        }
    } // namespace

    RenderWindow::RenderWindow(const RenderWindowDesc& desc)
        : window_(get_window_desc(desc))
        , device_(desc.device)
        , swapchain_desc_(get_swapchain_desc(desc))
        , swapchain_(desc.device->create_swapchain(window_, swapchain_desc_))
        , render_pass_(create_render_pass())
    {
        window_.on_resize_end().subscribe([this](const events::WindowResizeEnd& resize_end) {
            swapchain_desc_.image_size = resize_end.size;
            swapchain_->resize_images(swapchain_desc_);
            release_resources();
            create_image_views();
            create_framebuffers();
        });
    }

    RenderPassHandle RenderWindow::create_render_pass() const
    {
        const auto color_attachments = std::array{
            AttachmentDesc{
                .format = Format::B8G8R8A8_Srgb,
                .load_op = AttachmentLoadOp::DontCare,
                .store_op = AttachmentStoreOp::Store,
                .initial_layout = ImageLayout::Undefined,
                .layout = ImageLayout::ColorAttachment,
                .final_layout = ImageLayout::PresentSrc,
            },
        };
        const auto input_attachments = std::array{
            AttachmentDesc{
                .format = Format::B8G8R8A8_Srgb,
                .load_op = AttachmentLoadOp::Load,
                .store_op = AttachmentStoreOp::Store,
                .initial_layout = ImageLayout::Undefined,
                .layout = ImageLayout::ShaderReadOnly,
                .final_layout = ImageLayout::ColorAttachment,
            },
        };
        return device_->create_render_pass({
            .color_attachments = color_attachments,
            .input_attachments = input_attachments,
            .bind_point = PipelineBindPoint::Graphics,
        });
    }

    void RenderWindow::create_image_views()
    {
        ORION_ASSERT(image_views_.empty());
        image_views_.reserve(swapchain_desc_.image_count);
        for (std::uint32_t image_index = 0; image_index < swapchain_desc_.image_count; ++image_index) {
            image_views_.push_back(device_->create_image_view({
                .image = swapchain_->get_image(image_index),
                .type = ImageViewType::View2D,
                .format = swapchain_desc_.image_format,
            }));
        }
    }

    void RenderWindow::create_framebuffers()
    {
        ORION_ASSERT(framebuffers_.empty());
        framebuffers_.reserve(swapchain_desc_.image_count);
        for (auto image_view : image_views_) {
            framebuffers_.push_back(device_->create_framebuffer({
                .render_pass = render_pass_,
                .image_views = {{image_view}},
                .size = swapchain_desc_.image_size,
            }));
        }
    }

    void RenderWindow::release_resources()
    {
        device_->wait_idle();
        for (auto framebuffer : framebuffers_) {
            device_->destroy(framebuffer);
        }
        framebuffers_.clear();
        for (auto image_view : image_views_) {
            device_->destroy(image_view);
        }
        image_views_.clear();
    }
} // namespace orion
