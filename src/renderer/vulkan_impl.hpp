#pragma once

#include <volk.h>

#include <tl/expected.hpp>

#include <array>
#include <cstdint>
#include <vector>

namespace orion
{
    struct VulkanSemaphore {
        VkDevice vk_device = VK_NULL_HANDLE;
        VkSemaphore vk_semaphore = VK_NULL_HANDLE;

        VulkanSemaphore() = default;
        VulkanSemaphore(VkDevice device, VkSemaphore semaphore);
        VulkanSemaphore(const VulkanSemaphore&) = delete;
        VulkanSemaphore& operator=(const VulkanSemaphore&) = delete;
        VulkanSemaphore(VulkanSemaphore&& other) noexcept;
        VulkanSemaphore& operator=(VulkanSemaphore&& other) noexcept;
        ~VulkanSemaphore();

        tl::expected<void, VkResult> wait(std::uint64_t value, std::uint64_t timeout);
    };

    struct VulkanCommandPool {
        static constexpr auto max_command_buffers = 8;

        VkDevice vk_device = VK_NULL_HANDLE;
        VkCommandPool vk_command_pool = VK_NULL_HANDLE;
        std::array<VkCommandBuffer, max_command_buffers> vk_command_buffers = {};
        decltype(vk_command_buffers.begin()) current_command_buffer = vk_command_buffers.begin();

        VulkanCommandPool() = default;
        VulkanCommandPool(
            VkDevice device,
            VkCommandPool command_pool,
            std::array<VkCommandBuffer, max_command_buffers> command_buffers);
        VulkanCommandPool(const VulkanCommandPool&) = delete;
        VulkanCommandPool& operator=(const VulkanCommandPool&) = delete;
        VulkanCommandPool(VulkanCommandPool&& other) noexcept;
        VulkanCommandPool& operator=(VulkanCommandPool&& other) noexcept;
        ~VulkanCommandPool();

        tl::expected<void, VkResult> reset();
        tl::expected<VkCommandBuffer, VkResult> begin_command_buffer();
    };

    struct VulkanCommandPoolDesc {
        std::uint32_t queue_family_index;
        VkCommandPoolCreateFlags flags;
    };

    struct VulkanSwapchain {
        static constexpr auto max_image_count = 3u;

        VkDevice vk_device;
        VkSwapchainKHR vk_swapchain;

        std::uint32_t image_count;
        VkExtent2D image_extent;
        VkFormat image_format;
        VkPresentModeKHR present_mode;

        std::vector<VkImage> vk_images;
        std::vector<VkImageView> vk_image_views;

        VulkanSwapchain(
            VkDevice device,
            VkSwapchainKHR swapchain,
            std::uint32_t image_count,
            VkExtent2D image_extent,
            VkFormat image_format,
            VkPresentModeKHR present_mode,
            std::vector<VkImage> images,
            std::vector<VkImageView> image_views);
        VulkanSwapchain(const VulkanSwapchain&) = delete;
        VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
        VulkanSwapchain(VulkanSwapchain&& other) noexcept;
        VulkanSwapchain& operator=(VulkanSwapchain&& other) noexcept;
        ~VulkanSwapchain();

        tl::expected<std::uint32_t, VkResult> acquire_next_image(const VulkanSemaphore& semaphore, std::uint64_t timeout);
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
        VkQueue graphics_queue;

        VulkanDevice(
            VkDevice device,
            VkPhysicalDevice physical_device,
            VkInstance instance,
            std::uint32_t graphics_queue_family,
            VkQueue graphics_queue);
        VulkanDevice(const VulkanDevice&) = delete;
        VulkanDevice& operator=(const VulkanDevice&) = delete;
        VulkanDevice(VulkanDevice&& other) noexcept;
        VulkanDevice& operator=(VulkanDevice&& other) noexcept;
        ~VulkanDevice();

        tl::expected<VulkanSurface, VkResult> create_surface(const class Window& window);
        tl::expected<VulkanSwapchain, VkResult> create_swapchain(const VulkanSwapchainDesc& desc);
        tl::expected<VulkanCommandPool, VkResult> create_command_pool(const VulkanCommandPoolDesc& desc);
        tl::expected<VulkanSemaphore, VkResult> create_binary_semaphore();
        tl::expected<VulkanSemaphore, VkResult> create_timeline_semaphore(std::uint64_t initial_value);

        tl::expected<void, VkResult> wait_idle();
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
