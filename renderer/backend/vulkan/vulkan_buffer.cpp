#include "vulkan_buffer.h"

#include <utility>         // std::exchange

namespace orion::vulkan
{
    VulkanBuffer::VulkanBuffer(VmaAllocator allocator, VkBuffer buffer, VmaAllocation allocation)
        : vma_allocator_(allocator)
        , vk_buffer_(buffer)
        , vma_allocation_(allocation)
    {
    }

    VulkanBuffer::VulkanBuffer(VulkanBuffer&& other) noexcept
        : vma_allocator_(other.vma_allocator_)
        , vk_buffer_(std::exchange(other.vk_buffer_, VK_NULL_HANDLE))
        , vma_allocation_(std::exchange(other.vma_allocation_, VK_NULL_HANDLE))
    {
    }

    VulkanBuffer& VulkanBuffer::operator=(VulkanBuffer&& other) noexcept
    {
        vma_allocator_ = other.vma_allocator_;
        vk_buffer_ = std::exchange(other.vk_buffer_, VK_NULL_HANDLE);
        vma_allocation_ = std::exchange(other.vma_allocation_, VK_NULL_HANDLE);
        return *this;
    }

    VulkanBuffer::~VulkanBuffer()
    {
        if (vk_buffer_ != VK_NULL_HANDLE && vma_allocation_ != VK_NULL_HANDLE) {
            vmaDestroyBuffer(vma_allocator_, vk_buffer_, vma_allocation_);
        }
    }

    VulkanBuffer VulkanBuffer::create(VmaAllocator allocator, const BufferCreateInfo& create_info)
    {
        // Buffer create info
        const VkBufferCreateInfo buffer_info{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = create_info.size,
            .usage = create_info.usage,
            .sharingMode = create_info.queue_indices.size() > 1 ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = static_cast<uint32_t>(create_info.queue_indices.size()),
            .pQueueFamilyIndices = create_info.queue_indices.data(),
        };

        // Allocation info
        const VmaAllocationCreateInfo allocation_info{
            .flags = create_info.host_visible ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT : VmaAllocationCreateFlags{},
            .usage = VMA_MEMORY_USAGE_AUTO,
            .requiredFlags = 0,
            .preferredFlags = 0,
            .pool = VK_NULL_HANDLE,
            .pUserData = nullptr,
            .priority = 0.f,
        };

        // Create the buffer
        VkBuffer buffer = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;
        vk_result_check(vmaCreateBuffer(allocator, &buffer_info, &allocation_info, &buffer, &allocation, nullptr));
        return {allocator, buffer, allocation};
    }
} // namespace orion::vulkan
