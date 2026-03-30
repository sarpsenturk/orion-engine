#pragma once

#include <volk.h>

#include <tl/expected.hpp>

#include <cstdint>
#include <vector>

namespace orion
{
    struct VulkanSwapchain {
        static constexpr auto max_image_count = 3u;

        VkDevice vk_device;
        VkSwapchainKHR vk_swapchain;

        std::uint32_t image_count;
        VkExtent2D image_extent;
        VkFormat image_format;
        VkPresentModeKHR present_mode;

        VulkanSwapchain(
            VkDevice device,
            VkSwapchainKHR swapchain,
            std::uint32_t image_count,
            VkExtent2D image_extent,
            VkFormat image_format,
            VkPresentModeKHR present_mode);
        VulkanSwapchain(const VulkanSwapchain&) = delete;
        VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
        VulkanSwapchain(VulkanSwapchain&& other) noexcept;
        VulkanSwapchain& operator=(VulkanSwapchain&& other) noexcept;
        ~VulkanSwapchain();
    };

    struct VulkanSurface {
        VkInstance vk_instance;
        VkPhysicalDevice vk_physical_device;
        VkSurfaceKHR vk_surface;

        VulkanSurface(VkInstance instance, VkPhysicalDevice physical_device, VkSurfaceKHR surface);
        VulkanSurface(const VulkanSurface&) = delete;
        VulkanSurface& operator=(const VulkanSurface&) = delete;
        VulkanSurface(VulkanSurface&& other) noexcept;
        VulkanSurface& operator=(VulkanSurface&& other) noexcept;
        ~VulkanSurface();

        [[nodiscard]] tl::expected<VkSurfaceCapabilitiesKHR, VkResult> surface_capabilities() const;
        [[nodiscard]] tl::expected<std::vector<VkSurfaceFormatKHR>, VkResult> surface_formats() const;
        [[nodiscard]] tl::expected<std::vector<VkPresentModeKHR>, VkResult> present_modes() const;
    };

    struct VulkanSwapchainDesc {
        const VulkanSurface& surface;
        VkExtent2D requested_extent;
        std::uint32_t requested_image_count;
        VkFormat requested_image_format;
        VkPresentModeKHR requested_present_mode;
        VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;
    };

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

        tl::expected<VulkanSurface, VkResult> create_surface(const class Window& window);
        tl::expected<VulkanSwapchain, VkResult> create_swapchain(const VulkanSwapchainDesc& desc);
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
