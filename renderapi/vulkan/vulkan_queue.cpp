#include "vulkan_queue.h"

#include "vulkan_command.h"
#include "vulkan_resource.h"

#include "orion-utils/assertion.h"

namespace orion::vulkan
{
    VulkanQueue::VulkanQueue(VkDevice device, VkQueue queue, std::uint32_t family, VulkanResourceManager* resource_manager)
        : device_(device)
        , queue_(queue)
        , family_(family)
        , resource_manager_(resource_manager)
    {
    }

    void VulkanQueue::vk_queue_submit(VkFence fence)
    {
        const auto submit = VkSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = nullptr,
            .waitSemaphoreCount = static_cast<uint32_t>(wait_semaphores_.size()),
            .pWaitSemaphores = wait_semaphores_.data(),
            .pWaitDstStageMask = wait_stages_.data(),
            .commandBufferCount = static_cast<uint32_t>(command_buffers_.size()),
            .pCommandBuffers = command_buffers_.data(),
            .signalSemaphoreCount = static_cast<uint32_t>(signal_semaphores_.size()),
            .pSignalSemaphores = signal_semaphores_.data(),
        };
        vk_result_check(vkQueueSubmit(queue_, 1, &submit, fence));

        wait_semaphores_.clear();
        wait_stages_.clear();
        command_buffers_.clear();
        signal_semaphores_.clear();
    }

    void VulkanQueue::add_command_buffers(std::span<const CommandList* const> command_lists)
    {
        command_buffers_.resize(command_lists.size());
        std::ranges::transform(command_lists, command_buffers_.begin(), [](const CommandList* command_list) {
            return static_cast<const VulkanCommandList*>(command_list)->vk_command_buffer();
        });
    }

    void VulkanQueue::wait_api(SemaphoreHandle semaphore)
    {
        VkSemaphore vk_semaphore = resource_manager_->find(semaphore);
        ORION_ASSERT(vk_semaphore != VK_NULL_HANDLE);
        wait_semaphores_.push_back(vk_semaphore);
        wait_stages_.push_back(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT); // TODO: Get more clever with this somehow?
    }

    void VulkanQueue::signal_api(SemaphoreHandle semaphore)
    {
        VkSemaphore vk_semaphore = resource_manager_->find(semaphore);
        ORION_ASSERT(vk_semaphore != VK_NULL_HANDLE);
        signal_semaphores_.push_back(vk_semaphore);
    }

    void VulkanQueue::submit_api(std::span<const CommandList* const> command_lists, FenceHandle signal_fence)
    {
        add_command_buffers(command_lists);
        vk_queue_submit(resource_manager_->find(signal_fence)); // Fence is allowed to be VK_NULL_HANDLE
    }

    void VulkanQueue::submit_immediate_api(std::span<const CommandList* const> command_lists)
    {
        // Create temporary fence
        VkFence immediate_fence = VK_NULL_HANDLE;
        {
            const auto fence_info = VkFenceCreateInfo{
                .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
            };
            vk_result_check(vkCreateFence(device_, &fence_info, alloc_callbacks(), &immediate_fence));
        }

        // Add command buffers and submit
        add_command_buffers(command_lists);
        vk_queue_submit(immediate_fence);

        // Wait CPU side until fence is set and destroy it
        vkWaitForFences(device_, 1, &immediate_fence, VK_TRUE, UINT64_MAX);
        vkDestroyFence(device_, immediate_fence, alloc_callbacks());
    }
} // namespace orion::vulkan
