#pragma once

#include "orion/renderapi/render_queue.h"
#include "vulkan_context.h"

#include <Volk/volk.h>

#include <vector>

namespace orion
{
    class VulkanQueue final : public CommandQueue
    {
    public:
        VulkanQueue(VkQueue queue, VulkanContext* context);

        void vk_queue_wait(VkSemaphore semaphore, VkPipelineStageFlags wait_stages);
        void vk_queue_signal(VkSemaphore semaphore);

        VkQueue vk_queue() const { return queue_; }

    private:
        void wait_api(SemaphoreHandle semaphore) override;
        void signal_api(SemaphoreHandle semaphore) override;

        void submit_api(std::span<const class CommandList* const> command_lists, FenceHandle fence) override;

        VkQueue queue_;
        VulkanContext* context_;

        // Store command buffers in the object
        // to avoid reallocation every time submit is called
        std::vector<VkCommandBuffer> command_buffers_;
        std::vector<VkSemaphore> wait_semaphores_;
        std::vector<VkPipelineStageFlags> wait_stages_;
        std::vector<VkSemaphore> signal_semaphores_;
    };
} // namespace orion
