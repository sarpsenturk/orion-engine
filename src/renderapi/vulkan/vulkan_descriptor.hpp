#pragma once

#include <Volk/volk.h>

namespace orion
{
    struct VulkanDescriptorSetLayout {
        VulkanDescriptorSetLayout() = default;
        VulkanDescriptorSetLayout(VkDescriptorSetLayout _layout, VkDescriptorPool _pool);
        VulkanDescriptorSetLayout(std::nullptr_t) {}

        operator bool() const noexcept { return layout != VK_NULL_HANDLE; }

        VkDescriptorSetLayout layout = VK_NULL_HANDLE;
        VkDescriptorPool pool = VK_NULL_HANDLE;
    };
} // namespace orion
