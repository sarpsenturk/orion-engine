#pragma once

#include <volk.h>

#include <tl/expected.hpp>

#include <cstdint>

namespace orion
{
    struct VulkanDevice {
        VkDevice vk_device;
        VkPhysicalDevice vk_physical_device;
        VkInstance vk_instance;

        std::uint32_t graphics_queue_family;

        VulkanDevice(
            VkDevice device,
            VkPhysicalDevice physical_device,
            VkInstance instance,
            std::uint32_t graphics_queue_family);
        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice& operator=(const VulkanDevice&) = delete;
        VulkanDevice(VulkanDevice&& other) noexcept;
        VulkanDevice& operator=(VulkanDevice&& other) noexcept;
        ~VulkanDevice();
    };

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

        tl::expected<std::vector<VkPhysicalDevice>, VkResult> enumerate_physical_devices();
        tl::expected<VulkanDevice, VkResult> create_device(VkPhysicalDevice physical_device);
    };
} // namespace orion
