#include "vulkan_render_context.h"

namespace orion::vulkan
{
    VulkanRenderContext::VulkanRenderContext(UniqueVkCommandPool command_pool)
        : device_(command_pool.get_deleter().device)
        , command_pool_(std::move(command_pool))
        , command_buffer_(allocate_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY))
    {
    }

    VkCommandBuffer VulkanRenderContext::allocate_command_buffer(VkCommandBufferLevel level)
    {
        const VkCommandBufferAllocateInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = *command_pool_,
            .level = level,
            .commandBufferCount = 1,
        };
        VkCommandBuffer command_buffer = VK_NULL_HANDLE;
        vk_result_check(vkAllocateCommandBuffers(device_, &info, &command_buffer));
        return command_buffer;
    }

    std::vector<VkCommandBuffer> VulkanRenderContext::allocate_command_buffers(std::uint32_t count, VkCommandBufferLevel level)
    {
        std::vector<VkCommandBuffer> command_buffers(count);
        const VkCommandBufferAllocateInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = *command_pool_,
            .level = level,
            .commandBufferCount = count,
        };
        vk_result_check(vkAllocateCommandBuffers(device_, &info, command_buffers.data()));
        return command_buffers;
    }
} // namespace orion::vulkan
