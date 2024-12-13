#pragma once

#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

namespace orion
{
    struct VulkanBuffer {
        VulkanBuffer() = default;

        VulkanBuffer(VkBuffer _buffer, VmaAllocation _allocation)
            : buffer(_buffer)
            , allocation(_allocation)
        {
        }

        VulkanBuffer(std::nullptr_t) {}

        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        explicit(false) operator bool() const noexcept { return buffer != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE; }
    };
} // namespace orion
