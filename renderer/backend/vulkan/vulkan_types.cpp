#include "vulkan_types.h"

#include "orion-core/config.h"

#include <algorithm>               // std::ranges::find_if
#include <orion-utils/assertion.h> // ORION_EXPECTS
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
        vkFreeDescriptorSets(device, pool, 1, &descriptor_set);
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

    UniqueVkSwapchainKHR vulkan::unique(VkSwapchainKHR swapchain, VkDevice device)
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

    UniqueVkDescriptorPool unique(VkDescriptorPool descriptor_pool, VkDevice device)
    {
        return UniqueVkDescriptorPool{descriptor_pool, DescriptorPoolDeleter{device}};
    }

    UniqueVkDescriptorSet unique(VkDescriptorSet descriptor_set, VkDevice device, VkDescriptorPool descriptor_pool)
    {
        return UniqueVkDescriptorSet{descriptor_set, DescriptorSetDeleter{device, descriptor_pool}};
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
        return unique(semaphore, device);
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
        return unique(fence, device);
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
        vk_result_check(
            vkAcquireNextImageKHR(device_, swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &available_image),
            {VK_SUCCESS, VK_TIMEOUT, VK_NOT_READY, VK_SUBOPTIMAL_KHR});

        swapchain_->set_image_index(available_image);
        return framebuffers_[available_image].get();
    }
} // namespace orion::vulkan
