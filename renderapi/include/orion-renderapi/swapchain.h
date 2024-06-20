#pragma once

#include "orion-renderapi/format.h"
#include "orion-renderapi/image.h"

#include <cstdint>
#include <memory>

namespace orion
{
    struct SwapchainDesc {
        std::uint32_t width;
        std::uint32_t height;
        Format image_format;
        std::uint32_t image_count;
        ImageUsageFlags image_usage;
    };

    class Swapchain
    {
    public:
        Swapchain() = default;
        virtual ~Swapchain() = default;

        ImageViewHandle acquire_render_target();
        ImageHandle current_image();
        void present(std::uint32_t sync_interval);

    protected:
        Swapchain(const Swapchain&) = default;
        Swapchain(Swapchain&&) = default;
        Swapchain& operator=(const Swapchain&) = default;
        Swapchain& operator=(Swapchain&&) = default;

    private:
        virtual ImageViewHandle acquire_render_target_api() = 0;
        virtual ImageHandle current_image_api() = 0;
        virtual void present_api(std::uint32_t sync_interval) = 0;
    };

    using SwapchainPtr = std::unique_ptr<Swapchain>;
} // namespace orion
