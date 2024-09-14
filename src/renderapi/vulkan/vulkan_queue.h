#pragma once

#include "orion/renderapi/render_queue.h"

#include <Volk/volk.h>

#include <vector>

namespace orion
{
    class VulkanQueue final : public CommandQueue
    {
    public:
        VulkanQueue(VkQueue queue);

        void vk_queue_wait(VkSemaphore semaphore, VkPipelineStageFlags wait_stages);
        void vk_queue_signal(VkSemaphore semaphore);

        VkQueue vk_queue() const { return queue_; }

    private:
        VkQueue queue_;

        std::vector<VkSemaphore> wait_semaphores_;
        std::vector<VkPipelineStageFlags> wait_stages_;
        std::vector<VkSemaphore> signal_semaphores_;
    };
} // namespace orion
