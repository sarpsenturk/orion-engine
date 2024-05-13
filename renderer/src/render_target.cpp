#include "orion-renderer/render_target.h"

#include "orion-renderapi/render_device.h"

#include "orion-math/vector/vector3.h"

#include <utility>

namespace orion
{
    RenderTarget::RenderTarget(UniqueImage image, UniqueImageView image_view, UniqueRenderPass render_pass, UniqueFramebuffer framebuffer)
        : image_(std::move(image))
        , image_view_(std::move(image_view))
        , render_pass_(std::move(render_pass))
        , framebuffer_(std::move(framebuffer))
    {
    }

    RenderTarget create_render_target(RenderDevice* device, const RenderTargetDesc& desc)
    {
        auto image = device->create_image({
            .type = ImageType::Image2D,
            .format = Format::B8G8R8A8_Srgb,
            .size = vec3(desc.size, 1u),
            .tiling = ImageTiling::Optimal,
            .usage = ImageUsageFlags::ColorAttachment | ImageUsageFlags::SampledImage,
            .host_visible = false,
        });

        auto image_view = device->create_image_view({
            .image = image,
            .type = ImageViewType::View2D,
            .format = Format::B8G8R8A8_Srgb,
        });

        auto render_pass = device->create_render_pass({
            .color_attachments = {{
                AttachmentDesc{
                    .format = Format::B8G8R8A8_Srgb,
                    .load_op = AttachmentLoadOp::Clear,
                    .store_op = AttachmentStoreOp::Store,
                    .initial_layout = ImageLayout::Undefined,
                    .layout = ImageLayout::ColorAttachment,
                    .final_layout = ImageLayout::ShaderReadOnly,
                },
            }},
        });

        auto framebuffer = device->create_framebuffer({
            .render_pass = render_pass,
            .image_views = {&image_view, 1},
            .size = desc.size,
        });

        return RenderTarget{device->to_unique(image), device->to_unique(image_view), device->to_unique(render_pass), device->to_unique(framebuffer)};
    }
} // namespace orion
