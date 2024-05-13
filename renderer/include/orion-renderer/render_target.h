#pragma once

#include "orion-renderapi/defs.h"
#include "orion-renderapi/device_resource.h"

#include "orion-math/vector/vector2.h"

#include <variant>

namespace orion
{
    using RenderTargetImage = std::variant<ImageHandle, UniqueImage>;
    ImageHandle get_image_handle(const RenderTargetImage& render_target_image);

    class RenderTarget
    {
    public:
        RenderTarget(RenderTargetImage image, UniqueImageView image_view, UniqueRenderPass render_pass, UniqueFramebuffer framebuffer);

        [[nodiscard]] ImageViewHandle image_view() const noexcept { return image_view_.get(); }
        [[nodiscard]] FramebufferHandle framebuffer() const noexcept { return framebuffer_.get(); }
        [[nodiscard]] RenderPassHandle render_pass() const noexcept { return render_pass_.get(); }

    private:
        RenderTargetImage image_;
        UniqueImageView image_view_;
        UniqueRenderPass render_pass_;
        UniqueFramebuffer framebuffer_;
    };

    struct RenderTargetDesc {
        Vector2_u size;
        ImageUsageFlags image_usage = ImageUsageFlags::ColorAttachment;
        ImageLayout initial_layout = ImageLayout::Undefined;
        ImageLayout final_layout = ImageLayout::General;
    };

    // Forward declare
    class RenderDevice;

    RenderTarget create_render_target(RenderDevice* device, ImageHandle image, const RenderTargetDesc& desc);
    RenderTarget create_render_target(RenderDevice* device, const RenderTargetDesc& desc);
} // namespace orion
