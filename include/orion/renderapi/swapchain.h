#pragma once

#include "orion/renderapi/format.h"
#include "orion/renderapi/handle.h"

#include <cstdint>

namespace orion
{
    struct SwapchainDesc {
        const class Window* window;
        class CommandQueue* queue;
        std::uint32_t width;
        std::uint32_t height;
        std::uint32_t image_count;
        Format image_format;
    };

    class Swapchain
    {
    public:
        Swapchain() = default;
        virtual ~Swapchain() = default;

        ImageViewHandle acquire_render_target();
        ImageHandle current_image();

        void present(bool vsync);

    protected:
        Swapchain(const Swapchain&) = default;
        Swapchain& operator=(const Swapchain&) = default;
        Swapchain(Swapchain&&) = default;
        Swapchain& operator=(Swapchain&&) = default;

    private:
        virtual ImageViewHandle acquire_render_target_api() = 0;
        virtual ImageHandle current_image_api() = 0;
        virtual void present_api(bool vsync) = 0;
    };
} // namespace orion
