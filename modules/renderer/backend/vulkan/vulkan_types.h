#pragma once

#include "vulkan_headers.h"

namespace orion::vulkan
{
    struct VulkanQueues {
        struct Queue {
            std::uint32_t index;
            VkQueue queue;
        };

        Queue graphics;
        Queue compute;
        Queue transfer;
    };
} // namespace orion::vulkan
