#include "vulkan_types.h"

#include <orion-utils/assertion.h> // ORION_EXPECTS
#include <spdlog/spdlog.h>         // SPDLOG_*

namespace orion::vulkan
{

    void InstanceDeleter::operator()(VkInstance instance) const
    {
        vkDestroyInstance(instance, alloc_callbacks());
    }

    void DeviceDeleter::operator()(VkDevice device) const
    {
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
} // namespace orion::vulkan
