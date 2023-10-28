#pragma once

#include "vulkan_headers.h"
#include "vulkan_types.h"

#include <vector>

namespace orion::vulkan
{
    class VulkanJob
    {
    public:
        VulkanJob(UniqueVkFence fence, UniqueVkSemaphore semaphore, std::vector<VkSemaphore> dependencies);

        [[nodiscard]] VkFence vk_fence() const noexcept { return fence_.get(); }
        [[nodiscard]] VkSemaphore vk_semaphore() const noexcept { return semaphore_.get(); }

    private:
        UniqueVkFence fence_;
        UniqueVkSemaphore semaphore_;
        std::vector<VkSemaphore> wait_semaphores_;
        std::vector<VkPipelineStageFlags> wait_stages_;
    };
} // namespace orion::vulkan
