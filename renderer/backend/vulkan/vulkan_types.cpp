#include "vulkan_types.h"

#include "orion-core/config.h"

#include <algorithm>               // std::ranges::find_if
#include <orion-utils/assertion.h> // ORION_EXPECTS
#include <spdlog/spdlog.h>         // SPDLOG_*
#include <vector>                  // std::vector

namespace orion::vulkan
{
    // Internal helpers
    namespace
    {
        bool check_extensions_supported(std::span<const char* const> enabled_extensions, std::span<const VkExtensionProperties> supported_extensions)
        {
            for (const char* extension : enabled_extensions) {
                const auto pred = [extension](const VkExtensionProperties& extension_properties) { return std::strcmp(extension, extension_properties.extensionName) == 0; };
                const auto supported = std::ranges::find_if(supported_extensions, pred) != supported_extensions.end();
                if (!supported) {
                    return false;
                }
            }
            return true;
        }

        std::vector<VkExtensionProperties> get_supported_device_extensions(VkPhysicalDevice physical_device)
        {
            std::uint32_t count = 0;
            vk_result_check(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr));
            std::vector<VkExtensionProperties> extensions(count);
            vk_result_check(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, extensions.data()));
            return extensions;
        }
    } // namespace

    void InstanceDeleter::operator()(VkInstance instance) const
    {
        vkDestroyInstance(instance, alloc_callbacks());
    }

    void DeviceDeleter::operator()(VkDevice device) const
    {
        vkDeviceWaitIdle(device);
        vkDestroyDevice(device, alloc_callbacks());
    }

    void DebugUtilsMessengerDeleter::operator()(VkDebugUtilsMessengerEXT debug_messenger) const
    {
        ORION_EXPECTS(instance != VK_NULL_HANDLE);
        static const auto pfn_vkDestroyDebugUtilsMessengerEXT = [this]() {
            return reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        }();
        pfn_vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, alloc_callbacks());
    }

    void SurfaceDeleter::operator()(VkSurfaceKHR surface) const
    {
        ORION_EXPECTS(instance != VK_NULL_HANDLE);
        vkDestroySurfaceKHR(instance, surface, alloc_callbacks());
    }

    void SwapchainDeleter::operator()(VkSwapchainKHR swapchain) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroySwapchainKHR(device, swapchain, alloc_callbacks());
    }

    void ImageViewDeleter::operator()(VkImageView image_view) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyImageView(device, image_view, alloc_callbacks());
    }

    void CommandPoolDeleter::operator()(VkCommandPool command_pool) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyCommandPool(device, command_pool, alloc_callbacks());
    }

    void CommandBufferDeleter::operator()(VkCommandBuffer command_buffer) const
    {
        ORION_ASSERT(device != VK_NULL_HANDLE && command_pool != VK_NULL_HANDLE);
        vkFreeCommandBuffers(device, command_pool, 1, &command_buffer);
    }

    void RenderPassDeleter::operator()(VkRenderPass render_pass) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyRenderPass(device, render_pass, alloc_callbacks());
    }

    void FramebufferDeleter::operator()(VkFramebuffer framebuffer) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyFramebuffer(device, framebuffer, alloc_callbacks());
    }

    void ShaderModuleDeleter::operator()(VkShaderModule shader_module) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyShaderModule(device, shader_module, alloc_callbacks());
    }

    void PipelineLayoutDeleter::operator()(VkPipelineLayout pipeline_layout) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyPipelineLayout(device, pipeline_layout, alloc_callbacks());
    }

    void PipelineDeleter::operator()(VkPipeline pipeline) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyPipeline(device, pipeline, alloc_callbacks());
    }

    void AllocatorDeleter::operator()(VmaAllocator allocator) const
    {
        vmaDestroyAllocator(allocator);
    }

    void SemaphoreDeleter::operator()(VkSemaphore semaphore) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroySemaphore(device, semaphore, alloc_callbacks());
    }

    void FenceDeleter::operator()(VkFence fence) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyFence(device, fence, alloc_callbacks());
    }

    UniqueVkSemaphore create_vk_semaphore(VkDevice device)
    {
        const VkSemaphoreCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };
        VkSemaphore semaphore = VK_NULL_HANDLE;
        vk_result_check(vkCreateSemaphore(device, &info, alloc_callbacks(), &semaphore));
        return {semaphore, SemaphoreDeleter{device}};
    }

    UniqueVkFence create_vk_fence(VkDevice device, VkFenceCreateFlags flags)
    {
        const VkFenceCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
        };
        VkFence fence = VK_NULL_HANDLE;
        vk_result_check(vkCreateFence(device, &info, alloc_callbacks(), &fence));
        return {fence, FenceDeleter{device}};
    }

    VulkanSwapchain::VulkanSwapchain(VkSurfaceKHR surface, UniqueVkSwapchainKHR swapchain, std::vector<UniqueVkImageView> image_views)
        : surface_(surface)
        , swapchain_(std::move(swapchain))
        , image_views_(std::move(image_views))
    {
    }

    VulkanSwapchainRenderTarget::VulkanSwapchainRenderTarget(VkDevice device,
                                                             VulkanSwapchain* swapchain,
                                                             std::vector<UniqueVkFramebuffer> framebuffers,
                                                             UniqueVkSemaphore semaphore)
        : device_(device)
        , swapchain_(swapchain)
        , framebuffers_(std::move(framebuffers))
        , semaphore_(std::move(semaphore))
    {
    }

    VkFramebuffer VulkanSwapchainRenderTarget::framebuffer() const
    {
        std::uint32_t available_image = 0;
        auto swapchain = swapchain_->swapchain();
        auto semaphore = semaphore_.get();
        auto result = vkAcquireNextImageKHR(device_, swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &available_image);
        vk_result_check(result, VK_SUCCESS, VK_TIMEOUT, VK_NOT_READY, VK_SUBOPTIMAL_KHR);

        swapchain_->set_image_index(available_image);
        return framebuffers_[available_image].get();
    }
} // namespace orion::vulkan
