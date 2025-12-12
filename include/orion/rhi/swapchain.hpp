#pragma once

#include "orion/rhi/format.hpp"
#include "orion/rhi/handle.hpp"

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
} // namespace orion
