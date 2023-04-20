#include "vulkan_types.h"

#include <orion-utils/assertion.h> // ORION_EXPECTS
#include <spdlog/spdlog.h>         // SPDLOG_*

namespace orion::vulkan
{

    void InstanceDeleter::operator()(VkInstance instance) const
    {
        vkDestroyInstance(instance, alloc_callbacks());
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Destroyed VkInstance {}", fmt::ptr(instance));
    }

    void DeviceDeleter::operator()(VkDevice device) const
    {
        vkDestroyDevice(device, alloc_callbacks());
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Destroyed VkDevice {}", fmt::ptr(device));
    }

    void DebugUtilsMessengerDeleter::operator()(VkDebugUtilsMessengerEXT debug_messenger) const
    {
        ORION_EXPECTS(instance != VK_NULL_HANDLE);
        static const auto pfn_vkDestroyDebugUtilsMessengerEXT = [this]() {
            return reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        }();
        pfn_vkDestroyDebugUtilsMessengerEXT(instance, debug_messenger, alloc_callbacks());
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Destroyed VkDebugUtilsMessengerEXT {}", fmt::ptr(debug_messenger));
    }

    void SurfaceDeleter::operator()(VkSurfaceKHR surface) const
    {
        ORION_EXPECTS(instance != VK_NULL_HANDLE);
        vkDestroySurfaceKHR(instance, surface, alloc_callbacks());
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Destroyed VkSurfaceKHR {}", fmt::ptr(surface));
    }

    void SwapchainDeleter::operator()(VkSwapchainKHR swapchain) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroySwapchainKHR(device, swapchain, alloc_callbacks());
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Destroyed VkSwapchainKHR {}", fmt::ptr(swapchain));
    }

    void ImageViewDeleter::operator()(VkImageView image_view) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyImageView(device, image_view, alloc_callbacks());
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Destroyed VkImageView {}", fmt::ptr(image_view));
    }

    void CommandPoolDeleter::operator()(VkCommandPool command_pool) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyCommandPool(device, command_pool, alloc_callbacks());
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Destroyed VkCommandPool {}", fmt::ptr(command_pool));
    }

    void RenderPassDeleter::operator()(VkRenderPass render_pass) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyRenderPass(device, render_pass, alloc_callbacks());
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Destroyed VkRenderPass {}", fmt::ptr(render_pass));
    }

    void FramebufferDeleter::operator()(VkFramebuffer framebuffer) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyFramebuffer(device, framebuffer, alloc_callbacks());
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Destroyed VkFramebuffer {}", fmt::ptr(framebuffer));
    }

    void ShaderModuleDeleter::operator()(VkShaderModule shader_module) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyShaderModule(device, shader_module, alloc_callbacks());
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Destroyed VkShaderModule {}", fmt::ptr(shader_module));
    }

    void PipelineLayoutDeleter::operator()(VkPipelineLayout pipeline_layout) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyPipelineLayout(device, pipeline_layout, alloc_callbacks());
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Destroyed VkPipelineLayout {}", fmt::ptr(pipeline_layout));
    }

    void PipelineDeleter::operator()(VkPipeline pipeline) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyPipeline(device, pipeline, alloc_callbacks());
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Destroyed VkPipeline {}", fmt::ptr(pipeline));
    }
} // namespace orion::vulkan
