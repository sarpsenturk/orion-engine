#include "vulkan_buffer.h"

#include <spdlog/spdlog.h> // SPDLOG_LOGGER_DEBUG
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
} // namespace orion::vulkan
