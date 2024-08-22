#pragma once

#include "orion/renderapi/format.h"

#include <cstdint>

namespace orion
{
    struct SwapchainDesc {
        const class Window* window;
        const class CommandQueue* queue;
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

    protected:
        Swapchain(const Swapchain&) = default;
        Swapchain& operator=(const Swapchain&) = default;
        Swapchain(Swapchain&&) = default;
        Swapchain& operator=(Swapchain&&) = default;
    };
} // namespace orion
