#include "vulkan_device.h"

#include "vulkan_conversion.h"
#include "vulkan_platform.h"
#include "vulkan_render_context.h"

#include <spdlog/spdlog.h> // SPDLOG_*
#include <utility>         // std::exchange

namespace orion::vulkan
{
    VulkanDevice::VulkanDevice(VkInstance instance, VkPhysicalDevice physical_device, UniqueVkDevice device, VulkanQueues queues)
        : instance_(instance)
        , physical_device_(physical_device)
        , device_(std::move(device))
        , queues_(queues)
    {
    }

    std::unique_ptr<RenderContext> VulkanDevice::create_render_context()
    {
        const VkCommandPoolCreateInfo command_pool_create_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
            .queueFamilyIndex = queues_.graphics.index,
        };
        VkCommandPool command_pool = VK_NULL_HANDLE;
        vk_result_check(vkCreateCommandPool(*device_, &command_pool_create_info, alloc_callbacks(), &command_pool));
        return std::make_unique<VulkanRenderContext>(UniqueVkCommandPool{command_pool, CommandPoolDeleter{*device_}});
    }

    SwapchainHandle VulkanDevice::create_swapchain_api(const Window& window, const SwapchainDesc& desc, SwapchainHandle existing)
    {
        // Create the surface
        auto surface = create_surface(instance_, window);

        // Chose present mode TODO: Allow user to select this
        const auto present_mode = VK_PRESENT_MODE_FIFO_KHR;

        // Get the surface capabilities
        const auto surface_capabilities = [this, &surface]() {
            VkSurfaceCapabilitiesKHR surface_capabilities;
            vk_result_check(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, *surface, &surface_capabilities));
            return surface_capabilities;
        }();

        // Get the old swapchain
        VkSwapchainKHR old_swapchain = [this, existing]() -> VkSwapchainKHR {
            if (existing.is_valid()) {
                return swapchains_.at(existing).swapchain();
            }
            return VK_NULL_HANDLE;
        }();

        // Create the swapchain
        const VkSwapchainCreateInfoKHR info{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .surface = *surface,
            .minImageCount = desc.image_count,
            .imageFormat = to_vulkan_type(desc.image_format),
            .imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR,
            .imageExtent = to_vulkan_extent(desc.image_size),
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = surface_capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = old_swapchain,
        };
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        vk_result_check(vkCreateSwapchainKHR(*device_, &info, alloc_callbacks(), &swapchain));
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Created VkSwapchainKHR {}", fmt::ptr(swapchain));

        // Insert into resources
        const auto handle = existing.is_valid() ? existing : SwapchainHandle::generate();
        swapchains_.insert_or_assign(handle, VulkanSwapchain{std::move(surface), UniqueVkSwapchainKHR{swapchain, {.device = *device_}}});

        return handle;
    }

    void VulkanDevice::destroy(SwapchainHandle swapchain_handle)
    {
        swapchains_.erase(swapchain_handle);
    }
} // namespace orion::vulkan
