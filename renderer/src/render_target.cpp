#include "orion-renderer/render_target.h"

#include "orion-renderapi/render_device.h"
#include "orion-renderapi/swapchain.h"

#include "orion-math/vector/vector3.h"

#include "orion-utils/callable.h"

#include <utility>

namespace orion
{
    namespace
    {
        RenderTarget make_render_target(RenderDevice* device, RenderTargetImage image, const RenderTargetDesc& desc)
        {
            auto image_view = device->create_image_view({
                .image = get_image_handle(image),
                .type = ImageViewType::View2D,
                .format = Format::B8G8R8A8_Srgb,
            });

            auto render_pass = device->create_render_pass({
                .color_attachments = {{
                    AttachmentDesc{
                        .format = Format::B8G8R8A8_Srgb,
                        .load_op = AttachmentLoadOp::Clear,
                        .store_op = AttachmentStoreOp::Store,
                        .initial_layout = desc.initial_layout,
                        .layout = ImageLayout::ColorAttachment,
                        .final_layout = desc.final_layout,
                    },
                }},
            });

            auto framebuffer = device->create_framebuffer({
                .render_pass = render_pass,
                .image_views = {&image_view, 1},
                .size = desc.size,
            });

            return RenderTarget{std::move(image), device->to_unique(image_view), device->to_unique(render_pass), device->to_unique(framebuffer)};
        }
    } // namespace

    ImageHandle get_image_handle(const RenderTargetImage& render_target_image)
    {
        const auto overload = Overload{
            [](const UniqueImage& image) { return image.get(); },
            [](ImageHandle image) { return image; },
        };
        return std::visit(overload, render_target_image);
    }

    RenderTarget::RenderTarget(RenderTargetImage image, UniqueImageView image_view, UniqueRenderPass render_pass, UniqueFramebuffer framebuffer)
        : image_(std::move(image))
        , image_view_(std::move(image_view))
        , render_pass_(std::move(render_pass))
        , framebuffer_(std::move(framebuffer))
    {
    }

    RenderTarget RenderTarget::create(RenderDevice* device, ImageHandle image, const RenderTargetDesc& desc)
    {
        return make_render_target(device, image, desc);
    }

    RenderTarget RenderTarget::create(RenderDevice* device, const RenderTargetDesc& desc)
    {
        auto image = device->create_image({
            .type = ImageType::Image2D,
            .format = Format::B8G8R8A8_Srgb,
            .size = vec3(desc.size, 1u),
            .tiling = ImageTiling::Optimal,
            .usage = desc.image_usage,
            .host_visible = false,
        });
        return make_render_target(device, device->to_unique(image), desc);
    }
} // namespace orion
