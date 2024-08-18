#include "vulkan_queue.h"

namespace orion
{
    VulkanQueue::VulkanQueue(VkQueue queue)
        : queue_(queue)
    {
    }
} // namespace orion
