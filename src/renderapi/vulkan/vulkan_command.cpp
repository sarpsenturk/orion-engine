#include "vulkan_command.h"

#include "vulkan_error.h"

#include "orion/assertion.h"

namespace orion
{
    VulkanCommandList::VulkanCommandList(VkDevice device, VkCommandBuffer command_buffer)
        : device_(device)
        , command_buffer_(command_buffer)
    {
        ORION_ASSERT(device_ != VK_NULL_HANDLE);
        ORION_ASSERT(command_buffer_ != VK_NULL_HANDLE);
    }

    void VulkanCommandList::begin_api()
    {
    }
    void VulkanCommandList::end_api()
    {
    }

    VulkanCommandAllocator::VulkanCommandAllocator(VkDevice device, VkCommandPool command_pool)
        : device_(device)
        , command_pool_(command_pool)
    {
        ORION_ASSERT(device_ != VK_NULL_HANDLE);
        ORION_ASSERT(command_pool_ != VK_NULL_HANDLE);
    }

    VulkanCommandAllocator::~VulkanCommandAllocator()
    {
        ORION_ASSERT(command_pool_ != VK_NULL_HANDLE);
        vkDestroyCommandPool(device_, command_pool_, nullptr);
    }

    void VulkanCommandAllocator::reset_api()
    {
        vk_assert(vkResetCommandPool(device_, command_pool_, {}));
    }
} // namespace orion
