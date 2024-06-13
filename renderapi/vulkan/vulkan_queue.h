#pragma once

#include "orion-renderapi/render_queue.h"

#include "vulkan_headers.h"
#include "vulkan_types.h"

#include <span>
#include <vector>

namespace orion::vulkan
{
    class VulkanResourceManager;

    class VulkanQueue final : public CommandQueue
    {
    public:
        VulkanQueue(VkDevice device, VkQueue queue, std::uint32_t family, VulkanResourceManager* resource_manager);

        void vk_queue_submit(VkFence fence);

    private:
        void add_command_buffers(std::span<const CommandList* const> command_lists);

        void wait_api(SemaphoreHandle semaphore) override;
        void signal_api(SemaphoreHandle semaphore) override;
        void submit_api(std::span<const CommandList* const> command_lists, FenceHandle signal_fence) override;
        void submit_immediate_api(std::span<const CommandList* const> command_lists) override;

        VkDevice device_;
        VkQueue queue_;
        std::uint32_t family_;
        VulkanResourceManager* resource_manager_;

        std::vector<VkSemaphore> wait_semaphores_;
        std::vector<VkPipelineStageFlags> wait_stages_;
        std::vector<VkCommandBuffer> command_buffers_;
        std::vector<VkSemaphore> signal_semaphores_;
    };
} // namespace orion::vulkan
