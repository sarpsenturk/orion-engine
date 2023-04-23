#pragma once

#include "vulkan_headers.h"

#include <span> // std::span

namespace orion::vulkan
{
    struct BufferCreateInfo {
        std::size_t size = 0;
        VkBufferUsageFlags usage = {};
        std::span<const std::uint32_t> queue_indices = {};
        bool host_visible = false;
    };

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

        static VulkanBuffer create(VmaAllocator allocator, const BufferCreateInfo& create_info);

    private:
        VmaAllocator vma_allocator_;
        VkBuffer vk_buffer_;
        VmaAllocation vma_allocation_;
    };
} // namespace orion::vulkan
