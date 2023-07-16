#pragma once

#include "handles.h"
#include "types.h"

namespace orion
{
    inline constexpr auto default_swapchain_image_count = 2;
    inline constexpr auto default_swapchain_format = Format::B8G8R8A8_Srgb;

    struct SwapchainDesc {
        std::uint32_t image_count = default_swapchain_image_count;
        Format image_format = default_swapchain_format;
        Vector2_u image_size = {};
    };

    struct SwapchainAttachmentDesc {
        SwapchainHandle swapchain = SwapchainHandle::invalid_handle();
        Format format = default_swapchain_format;
    };
} // namespace orion
