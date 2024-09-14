#include "vulkan_queue.h"

namespace orion
{
    VulkanQueue::VulkanQueue(VkQueue queue)
        : queue_(queue)
    {
    }

    void VulkanQueue::vk_queue_wait(VkSemaphore semaphore, VkPipelineStageFlags wait_stages)
    {
        wait_semaphores_.push_back(semaphore);
        wait_stages_.push_back(wait_stages);
    }

    void VulkanQueue::vk_queue_signal(VkSemaphore semaphore)
    {
        signal_semaphores_.push_back(semaphore);
    }
} // namespace orion
