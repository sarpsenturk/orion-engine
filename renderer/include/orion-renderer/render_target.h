#pragma once

#include "orion-renderapi/device_resource.h"

#include "orion-math/vector/vector2.h"

namespace orion
{
    class RenderTarget
    {
    public:
        RenderTarget(UniqueImage image, UniqueImageView image_view, UniqueRenderPass render_pass, UniqueFramebuffer framebuffer);

        [[nodiscard]] ImageHandle image() const noexcept { return image_.get(); }
        [[nodiscard]] ImageViewHandle image_view() const noexcept { return image_view_.get(); }
        [[nodiscard]] FramebufferHandle framebuffer() const noexcept { return framebuffer_.get(); }
        [[nodiscard]] RenderPassHandle render_pass() const noexcept { return render_pass_.get(); }

    private:
        UniqueImage image_;
        UniqueImageView image_view_;
        UniqueRenderPass render_pass_;
        UniqueFramebuffer framebuffer_;
    };

    // Forward declare
    class RenderDevice;

    struct RenderTargetDesc {
        Vector2_u size;
    };

    RenderTarget create_render_target(RenderDevice* device, const RenderTargetDesc& desc);
} // namespace orion
