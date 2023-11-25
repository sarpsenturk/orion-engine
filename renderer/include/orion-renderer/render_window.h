#pragma once

#include "orion-core/window.h"

#include "orion-renderapi/defs.h"
#include "orion-renderapi/swapchain.h"

#include <vector>

namespace orion
{
    // Forward declare
    class RenderDevice;

    struct RenderWindowDesc {
        RenderDevice* device;

        std::string name = "Orion Window";
        WindowPosition position;
        WindowSize size;

        std::uint32_t image_count = default_swapchain_image_count;
        Format image_format = default_swapchain_format;
        ImageUsageFlags image_usage = ImageUsageFlags::ColorAttachment;
    };

    class RenderWindow
    {
    public:
        explicit RenderWindow(const RenderWindowDesc& desc);

        [[nodiscard]] auto& window() { return window_; }
        [[nodiscard]] auto& window() const { return window_; }

    private:
        [[nodiscard]] RenderPassHandle create_render_pass() const;
        void create_image_views();
        void create_framebuffers();
        void release_resources();

        Window window_;
        RenderDevice* device_;
        SwapchainDesc swapchain_desc_;
        std::unique_ptr<Swapchain> swapchain_;
        RenderPassHandle render_pass_;
        std::vector<ImageViewHandle> image_views_;
        std::vector<FramebufferHandle> framebuffers_;
    };
} // namespace orion
