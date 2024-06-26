#include "vulkan_types.h"

#include <algorithm>
#include <orion-utils/assertion.h>
#include <vector>

namespace orion::vulkan
{
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
        static const auto vkDestroyDebugUtilsMessengerEXT_fn = [this]() {
            return reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
        }();
        vkDestroyDebugUtilsMessengerEXT_fn(instance, debug_messenger, alloc_callbacks());
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
        ORION_EXPECTS(device != VK_NULL_HANDLE && command_pool != VK_NULL_HANDLE);
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

    void DescriptorSetLayoutDeleter::operator()(VkDescriptorSetLayout descriptor_set_layout) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyDescriptorSetLayout(device, descriptor_set_layout, alloc_callbacks());
    }

    void DescriptorPoolDeleter::operator()(VkDescriptorPool descriptor_pool) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroyDescriptorPool(device, descriptor_pool, alloc_callbacks());
    }

    void DescriptorSetDeleter::operator()(VkDescriptorSet descriptor_set) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        ORION_EXPECTS(pool != VK_NULL_HANDLE);
        if (free_descriptor_set) {
            vkFreeDescriptorSets(device, pool, 1, &descriptor_set);
        }
    }

    void BufferDeleter::operator()(VkBuffer buffer) const
    {
        ORION_EXPECTS(allocator != VK_NULL_HANDLE);
        ORION_EXPECTS(allocation != VK_NULL_HANDLE);
        vmaDestroyBuffer(allocator, buffer, allocation);
    }

    void ImageDeleter::operator()(VkImage image) const
    {
        if (allocation != VK_NULL_HANDLE) {
            ORION_EXPECTS(allocator != VK_NULL_HANDLE);
            vmaDestroyImage(allocator, image, allocation);
        }
    }

    void SamplerDeleter::operator()(VkSampler sampler) const
    {
        ORION_EXPECTS(device != VK_NULL_HANDLE);
        vkDestroySampler(device, sampler, alloc_callbacks());
    }

    UniqueVkInstance unique(VkInstance instance)
    {
        return UniqueVkInstance{instance};
    }

    UniqueVkDevice unique(VkDevice device)
    {
        return UniqueVkDevice{device};
    }

    UniqueVkDebugUtilsMessengerEXT unique(VkDebugUtilsMessengerEXT debug_messenger, VkInstance instance)
    {
        return UniqueVkDebugUtilsMessengerEXT{debug_messenger, DebugUtilsMessengerDeleter{instance}};
    }

    UniqueVkSurfaceKHR unique(VkSurfaceKHR surface, VkInstance instance)
    {
        return UniqueVkSurfaceKHR{surface, SurfaceDeleter{instance}};
    }

    UniqueVkSwapchainKHR unique(VkSwapchainKHR swapchain, VkDevice device)
    {
        return UniqueVkSwapchainKHR{swapchain, SwapchainDeleter{device}};
    }

    UniqueVkImageView unique(VkImageView image_view, VkDevice device)
    {
        return UniqueVkImageView{image_view, ImageViewDeleter{device}};
    }
    UniqueVkCommandPool unique(VkCommandPool command_pool, VkDevice device)
    {
        return UniqueVkCommandPool{command_pool, CommandPoolDeleter{device}};
    }

    UniqueVkCommandBuffer unique(VkCommandBuffer command_buffer, VkDevice device, VkCommandPool command_pool)
    {
        return UniqueVkCommandBuffer{command_buffer, CommandBufferDeleter{device, command_pool}};
    }

    UniqueVkRenderPass unique(VkRenderPass render_pass, VkDevice device)
    {
        return UniqueVkRenderPass{render_pass, RenderPassDeleter{device}};
    }

    UniqueVkFramebuffer unique(VkFramebuffer framebuffer, VkDevice device)
    {
        return UniqueVkFramebuffer{framebuffer, FramebufferDeleter{device}};
    }

    UniqueVkShaderModule unique(VkShaderModule shader_module, VkDevice device)
    {
        return UniqueVkShaderModule{shader_module, ShaderModuleDeleter{device}};
    }

    UniqueVkPipelineLayout unique(VkPipelineLayout pipeline_layout, VkDevice device)
    {
        return UniqueVkPipelineLayout{pipeline_layout, PipelineLayoutDeleter{device}};
    }

    UniqueVkPipeline unique(VkPipeline pipeline, VkDevice device)
    {
        return UniqueVkPipeline{pipeline, PipelineDeleter{device}};
    }

    UniqueVmaAllocator unique(VmaAllocator vma_allocator)
    {
        return UniqueVmaAllocator{vma_allocator};
    }

    UniqueVkSemaphore unique(VkSemaphore semaphore, VkDevice device)
    {
        return UniqueVkSemaphore{semaphore, SemaphoreDeleter{device}};
    }

    UniqueVkFence unique(VkFence fence, VkDevice device)
    {
        return UniqueVkFence{fence, FenceDeleter{device}};
    }

    UniqueVkDescriptorSetLayout unique(VkDescriptorSetLayout descriptor_set_layout, VkDevice device)
    {
        return UniqueVkDescriptorSetLayout{descriptor_set_layout, DescriptorSetLayoutDeleter{device}};
    }

    UniqueVkDescriptorPool unique(VkDescriptorPool descriptor_pool, VkDevice device, VkDescriptorPoolCreateFlags flags)
    {
        return UniqueVkDescriptorPool{descriptor_pool, DescriptorPoolDeleter{device, flags}};
    }

    UniqueVkDescriptorSet unique(VkDescriptorSet descriptor_set, VkDevice device, VkDescriptorPool descriptor_pool, bool free_descriptor_set)
    {
        return UniqueVkDescriptorSet{descriptor_set, DescriptorSetDeleter{device, descriptor_pool, free_descriptor_set}};
    }

    UniqueVkBuffer unique(VkBuffer buffer, VmaAllocator allocator, VmaAllocation allocation)
    {
        return UniqueVkBuffer{buffer, BufferDeleter{allocator, allocation}};
    }

    UniqueVkImage unique(VkImage image, VmaAllocator allocator, VmaAllocation allocation)
    {
        return UniqueVkImage{image, ImageDeleter{allocator, allocation}};
    }

    UniqueVkSampler unique(VkSampler sampler, VkDevice device)
    {
        return UniqueVkSampler{sampler, SamplerDeleter{device}};
    }
} // namespace orion::vulkan
