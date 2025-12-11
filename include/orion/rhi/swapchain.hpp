#pragma once

#include "orion/rhi/format.hpp"

#include <cstdint>

namespace orion
{
    struct RHISwapchainDesc {
        const struct Window* window;
        const class RHICommandQueue* queue;
        std::uint32_t width;
        std::uint32_t height;
        RHIFormat format;
        std::uint32_t image_count;
    };

    class RHISwapchain
    {
    public:
        RHISwapchain() = default;
        virtual ~RHISwapchain() = default;

    protected:
        RHISwapchain(const RHISwapchain&) = default;
        RHISwapchain& operator=(const RHISwapchain&) = default;
        RHISwapchain(RHISwapchain&&) = default;
        RHISwapchain& operator=(RHISwapchain&&) = default;
    };
} // namespace orion
