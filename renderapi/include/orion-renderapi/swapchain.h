#pragma once

#include "orion-renderapi/format.h"
#include "orion-renderapi/handles.h"
#include "orion-renderapi/image.h"

#include "orion-math/vector/vector2.h"

#include <cstdint>
#include <span>

namespace orion
{
    namespace defaults
    {
        inline constexpr auto swapchain_image_count = 2;
        inline constexpr auto swapchain_format = Format::B8G8R8A8_Srgb;
    } // namespace defaults

    struct SwapchainDesc {
        std::uint32_t image_count = defaults::swapchain_image_count;
        Format image_format = defaults::swapchain_format;
        Vector2_u image_size = {};
        ImageUsageFlags image_usage = ImageUsageFlags::ColorAttachment;
        bool vsync = true;
    };
} // namespace orion
