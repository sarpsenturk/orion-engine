#pragma once

#include "vulkan_headers.h"

namespace orion::vulkan
{
    class VulkanBuffer
    {
    public:
        VulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation);
        VulkanBuffer(const VulkanBuffer&) = delete;
        VulkanBuffer(VulkanBuffer&& other) noexcept;
        VulkanBuffer& operator=(const VulkanBuffer&) = delete;
        VulkanBuffer& operator=(VulkanBuffer&& other) noexcept;
        ~VulkanBuffer();

        [[nodiscard]] auto buffer() const noexcept { return vk_buffer_; }
        [[nodiscard]] auto allocation() const noexcept { return vma_allocation_; }

    private:
        VmaAllocator vma_allocator_;
        VkBuffer vk_buffer_;
        VmaAllocation vma_allocation_;
    };
} // namespace orion::vulkan
