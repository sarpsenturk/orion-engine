#include "vulkan_device.h"

#include "vulkan_error.h"
#include "vulkan_platform.h"
#include "vulkan_queue.h"
#include "vulkan_swapchain.h"

#include "orion/assertion.h"

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace orion
{
    VulkanDevice::VulkanDevice(VkDevice device, VkInstance instance, VkPhysicalDevice physical_device, VkQueue queue, std::uint32_t queue_family_index)
        : device_(device)
        , instance_(instance)
        , physical_device_(physical_device)
        , queue_(queue)
        , queue_family_index_(queue_family_index)
    {
    }

    VulkanDevice::~VulkanDevice()
    {
        if (device_ != VK_NULL_HANDLE) {
            vkDestroyDevice(device_, nullptr);
        }
    }

    std::unique_ptr<CommandQueue> VulkanDevice::create_command_queue_api()
    {
        return std::make_unique<VulkanQueue>(queue_);
    }

    std::unique_ptr<Swapchain> VulkanDevice::create_swapchain_api(const SwapchainDesc& desc)
    {
        ORION_EXPECTS(desc.width != 0);
        ORION_EXPECTS(desc.height != 0);

        // Create the surface
        VkSurfaceKHR surface = create_platform_surface(instance_, desc.window);
        SPDLOG_TRACE("Created VkSurfaceKHR {}", fmt::ptr(surface));

        // Check if surface is supported
        {
            VkBool32 surface_supported = VK_FALSE;
            vk_assert(vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, queue_family_index_, surface, &surface_supported));
            if (!surface_supported) {
                throw std::runtime_error("Vulkan: physical device queue does not support surface");
            }
        }

        // Get surface capabilities
        VkSurfaceCapabilitiesKHR surface_capabilities;
        vk_assert(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface, &surface_capabilities));

        // Check requested swapchain against capabilities
        if (desc.image_count > surface_capabilities.maxImageCount) {
            throw std::runtime_error("Vulkan: requested swapchain image count is greater than VkSurfaceCapabilitiesKHR::maxImageCount");
        }

        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        {
            const auto info = VkSwapchainCreateInfoKHR{
                .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                .pNext = nullptr,
                .flags = 0,
                .surface = surface,
                .minImageCount = desc.image_count,
                .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,
                .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
                .imageExtent = {.width = desc.width, .height = desc.height},
                .imageArrayLayers = 1,
                .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .queueFamilyIndexCount = 0,
                .pQueueFamilyIndices = nullptr,
                .preTransform = surface_capabilities.currentTransform,
                .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                .presentMode = VK_PRESENT_MODE_FIFO_KHR,
                .clipped = VK_TRUE,
                .oldSwapchain = VK_NULL_HANDLE,
            };
            vk_assert(vkCreateSwapchainKHR(device_, &info, nullptr, &swapchain));
            SPDLOG_TRACE("Created VkSwapchainKHR {}", fmt::ptr(swapchain));
        }
        return std::make_unique<VulkanSwapchain>(device_, instance_, surface, swapchain);
    }
} // namespace orion
