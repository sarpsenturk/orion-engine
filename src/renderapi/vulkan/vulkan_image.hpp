#pragma once

#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>

namespace orion
{
    struct VulkanImage {
        VulkanImage() = default;

        VulkanImage(VkImage _image, VmaAllocation _allocation)
            : image(_image)
            , allocation(_allocation)
        {
        }

        VulkanImage(std::nullptr_t) {}

        VkImage image = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        explicit(false) operator bool() const noexcept { return image != VK_NULL_HANDLE; }

        [[nodiscard]] bool is_user_image() const noexcept { return image != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE; }
    };
} // namespace orion
