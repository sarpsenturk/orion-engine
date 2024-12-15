#include "vulkan_image.hpp"

namespace orion
{
    VulkanImage::VulkanImage(VkImage _image)
        : image(_image)
    {
    }

    VulkanImage::VulkanImage(VkImage _image, VmaAllocation _allocation)
        : image(_image)
        , allocation(_allocation)
    {
    }
} // namespace orion
