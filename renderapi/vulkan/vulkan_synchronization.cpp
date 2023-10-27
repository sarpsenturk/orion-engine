#include "vulkan_synchronization.h"

namespace orion::vulkan
{
    VulkanJob::VulkanJob(UniqueVkFence fence, UniqueVkSemaphore semaphore)
        : fence_(std::move(fence))
        , semaphore_(std::move(semaphore))
    {
    }

    void VulkanJob::wait_on(const VulkanJob& job, VkPipelineStageFlags wait_stages)
    {
        wait_semaphores_.push_back(job.vk_semaphore());
        wait_stages_.push_back(wait_stages);
    }

    void VulkanJob::reset()
    {
        wait_semaphores_.clear();
        wait_stages_.clear();
    }
} // namespace orion::vulkan
