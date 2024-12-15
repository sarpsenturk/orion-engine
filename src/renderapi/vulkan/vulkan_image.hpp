#pragma once

#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

namespace orion
{
    struct VulkanImage {
        VulkanImage() = default;

        explicit VulkanImage(VkImage _image);

        VulkanImage(VkImage _image, VmaAllocation _allocation);

        VulkanImage(std::nullptr_t) {}

        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        explicit(false) operator bool() const noexcept { return image != VK_NULL_HANDLE; }

        [[nodiscard]] bool is_user_image() const noexcept { return image != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE; }
    };
} // namespace orion
