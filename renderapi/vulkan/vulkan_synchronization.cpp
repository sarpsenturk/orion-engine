#include "vulkan_synchronization.h"

namespace orion::vulkan
{
    VulkanJob::VulkanJob(UniqueVkFence fence, UniqueVkSemaphore semaphore, std::vector<VkSemaphore> dependencies)
        : fence_(std::move(fence))
        , semaphore_(std::move(semaphore))
        , wait_semaphores_(std::move(dependencies))
        // TODO: We should find a way to not use VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
        //  and is still simple
        , wait_stages_(wait_semaphores_.size(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT)
    {
    }
} // namespace orion::vulkan
