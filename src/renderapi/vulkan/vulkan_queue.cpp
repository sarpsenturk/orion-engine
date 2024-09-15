#include "vulkan_queue.h"

#include "vulkan_command.h"
#include "vulkan_error.h"

#include "orion/assertion.h"

#include <spdlog/spdlog.h>

#include <algorithm>

namespace orion
{
    VulkanQueue::VulkanQueue(VkQueue queue, VulkanContext* context)
        : queue_(queue)
        , context_(context)
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

    void VulkanQueue::wait_api(SemaphoreHandle semaphore)
    {
        VkSemaphore vk_semaphore = context_->lookup(semaphore);
        ORION_ASSERT(vk_semaphore != VK_NULL_HANDLE);
        wait_semaphores_.push_back(vk_semaphore);
        wait_stages_.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    }

    void VulkanQueue::signal_api(SemaphoreHandle semaphore)
    {
        VkSemaphore vk_semaphore = context_->lookup(semaphore);
        ORION_ASSERT(vk_semaphore != VK_NULL_HANDLE);
        signal_semaphores_.push_back(vk_semaphore);
    }

    void VulkanQueue::submit_api(std::span<const class CommandList* const> command_lists, FenceHandle fence)
    {
        // Get VkCommandBuffers
        command_buffers_.reserve(command_lists.size());
        std::ranges::transform(command_lists, command_buffers_.begin(), [](const CommandList* command_list) {
            auto* vulkan_command_list = dynamic_cast<const VulkanCommandList*>(command_list);
            ORION_ASSERT(vulkan_command_list != nullptr);
            return vulkan_command_list->vk_command_buffer();
        });

        VkFence vk_fence = context_->lookup(fence);
        // Check if the fence was a valid handle if context returned VK_NULL_HANDLE
#ifdef ORION_BUILD_DEBUG
        if (vk_fence != VK_NULL_HANDLE && fence != FenceHandle()) {
            SPDLOG_ERROR("CommandQueue::submit(): fence was not FenceHandle::Invalid but no fence with handle {} found", fmt::underlying(fence));
            ORION_ASSERT(false);
        }
#endif

        // Submit command buffers
        const auto info = VkSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<std::uint32_t>(wait_semaphores_.size()),
            .pWaitSemaphores = wait_semaphores_.data(),
            .pWaitDstStageMask = wait_stages_.data(),
            .commandBufferCount = static_cast<std::uint32_t>(command_buffers_.size()),
            .pCommandBuffers = command_buffers_.data(),
            .signalSemaphoreCount = static_cast<std::uint32_t>(signal_semaphores_.size()),
            .pSignalSemaphores = signal_semaphores_.data(),
        };
        vk_assert(vkQueueSubmit(queue_, 1, &info, vk_fence));

        // Clear command buffers, wait semaphores & stages, signal semaphores for next submit
        command_buffers_.clear();
        wait_semaphores_.clear();
        wait_stages_.clear();
        signal_semaphores_.clear();
    }
} // namespace orion
