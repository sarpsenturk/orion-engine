#pragma once

#include <volk.h>

#include <tl/expected.hpp>

namespace orion
{
    struct VulkanInstance {
        VkInstance vk_instance;
        VkDebugUtilsMessengerEXT vk_debug_messenger;

        static tl::expected<VulkanInstance, VkResult> create();
        VulkanInstance(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger);
        VulkanInstance(const VulkanInstance&) = delete;
        VulkanInstance& operator=(const VulkanInstance&) = delete;
        VulkanInstance(VulkanInstance&& other) noexcept;
        VulkanInstance& operator=(VulkanInstance&& other) noexcept;
        ~VulkanInstance();
    };
} // namespace orion
