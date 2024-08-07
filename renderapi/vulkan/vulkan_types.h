#pragma once

#include "vulkan_headers.h"

#include <memory>
#include <span>

namespace orion::vulkan
{
    struct VulkanQueues {
        struct Queue {
            std::uint32_t family;
            VkQueue queue;
        };

        Queue graphics;
        Queue compute;
        Queue transfer;
    };

    struct InstanceDeleter {
        using pointer = VkInstance;

        void operator()(VkInstance instance) const;
    };

    struct DeviceDeleter {
        using pointer = VkDevice;

        void operator()(VkDevice device) const;
    };

    struct DebugUtilsMessengerDeleter {
        using pointer = VkDebugUtilsMessengerEXT;
        VkInstance instance = VK_NULL_HANDLE;

        void operator()(VkDebugUtilsMessengerEXT debug_messenger) const;
    };

    struct SurfaceDeleter {
        using pointer = VkSurfaceKHR;
        VkInstance instance = VK_NULL_HANDLE;

        void operator()(VkSurfaceKHR surface) const;
    };

    struct SwapchainDeleter {
        using pointer = VkSwapchainKHR;
        VkDevice device = VK_NULL_HANDLE;

        void operator()(VkSwapchainKHR swapchain) const;
    };

    struct ImageViewDeleter {
        using pointer = VkImageView;
        VkDevice device = VK_NULL_HANDLE;

        void operator()(VkImageView image_view) const;
    };

    struct CommandPoolDeleter {
        using pointer = VkCommandPool;
        VkDevice device = VK_NULL_HANDLE;

        void operator()(VkCommandPool command_pool) const;
    };

    struct CommandBufferDeleter {
        using pointer = VkCommandBuffer;
        VkDevice device = VK_NULL_HANDLE;
        VkCommandPool command_pool = VK_NULL_HANDLE;

        void operator()(VkCommandBuffer command_buffer) const;
    };

    struct RenderPassDeleter {
        using pointer = VkRenderPass;
        VkDevice device = VK_NULL_HANDLE;

        void operator()(VkRenderPass render_pass) const;
    };

    struct FramebufferDeleter {
        using pointer = VkFramebuffer;
        VkDevice device = VK_NULL_HANDLE;

        void operator()(VkFramebuffer framebuffer) const;
    };

    struct ShaderModuleDeleter {
        using pointer = VkShaderModule;
        VkDevice device = VK_NULL_HANDLE;

        void operator()(VkShaderModule shader_module) const;
    };

    struct PipelineLayoutDeleter {
        using pointer = VkPipelineLayout;
        VkDevice device = VK_NULL_HANDLE;

        void operator()(VkPipelineLayout pipeline_layout) const;
    };

    struct PipelineDeleter {
        using pointer = VkPipeline;
        VkDevice device = VK_NULL_HANDLE;

        void operator()(VkPipeline pipeline) const;
    };

    struct AllocatorDeleter {
        using pointer = VmaAllocator;

        void operator()(VmaAllocator allocator) const;
    };

    struct SemaphoreDeleter {
        using pointer = VkSemaphore;
        VkDevice device = VK_NULL_HANDLE;

        void operator()(VkSemaphore semaphore) const;
    };

    struct FenceDeleter {
        using pointer = VkFence;
        VkDevice device = VK_NULL_HANDLE;

        void operator()(VkFence fence) const;
    };

    struct DescriptorSetLayoutDeleter {
        using pointer = VkDescriptorSetLayout;
        VkDevice device = VK_NULL_HANDLE;

        void operator()(VkDescriptorSetLayout descriptor_set_layout) const;
    };

    struct DescriptorPoolDeleter {
        using pointer = VkDescriptorPool;

        VkDevice device = VK_NULL_HANDLE;
        VkDescriptorPoolCreateFlags flags = 0;

        void operator()(VkDescriptorPool descriptor_pool) const;
    };

    struct DescriptorSetDeleter {
        using pointer = VkDescriptorSet;

        VkDevice device = VK_NULL_HANDLE;
        VkDescriptorPool pool = VK_NULL_HANDLE;
        bool free_descriptor_set = false;

        void operator()(VkDescriptorSet descriptor_set) const;
    };

    struct BufferDeleter {
        using pointer = VkBuffer;

        VmaAllocator allocator = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        void operator()(VkBuffer buffer) const;
    };

    struct ImageDeleter {
        using pointer = VkImage;

        VmaAllocator allocator = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        void operator()(VkImage image) const;
    };

    struct SamplerDeleter {
        using pointer = VkSampler;

        VkDevice device = VK_NULL_HANDLE;

        void operator()(VkSampler sampler) const;
    };

    using UniqueVkInstance = std::unique_ptr<VkInstance, InstanceDeleter>;
    using UniqueVkDevice = std::unique_ptr<VkDevice, DeviceDeleter>;
    using UniqueVkDebugUtilsMessengerEXT = std::unique_ptr<VkDebugUtilsMessengerEXT, DebugUtilsMessengerDeleter>;
    using UniqueVkSurfaceKHR = std::unique_ptr<VkSurfaceKHR, SurfaceDeleter>;
    using UniqueVkSwapchainKHR = std::unique_ptr<VkSwapchainKHR, SwapchainDeleter>;
    using UniqueVkImageView = std::unique_ptr<VkImageView, ImageViewDeleter>;
    using UniqueVkCommandPool = std::unique_ptr<VkCommandPool, CommandPoolDeleter>;
    using UniqueVkCommandBuffer = std::unique_ptr<VkCommandBuffer, CommandBufferDeleter>;
    using UniqueVkRenderPass = std::unique_ptr<VkRenderPass, RenderPassDeleter>;
    using UniqueVkFramebuffer = std::unique_ptr<VkFramebuffer, FramebufferDeleter>;
    using UniqueVkShaderModule = std::unique_ptr<VkShaderModule, ShaderModuleDeleter>;
    using UniqueVkPipelineLayout = std::unique_ptr<VkPipelineLayout, PipelineLayoutDeleter>;
    using UniqueVkPipeline = std::unique_ptr<VkPipeline, PipelineDeleter>;
    using UniqueVmaAllocator = std::unique_ptr<VmaAllocator, AllocatorDeleter>;
    using UniqueVkSemaphore = std::unique_ptr<VkSemaphore, SemaphoreDeleter>;
    using UniqueVkFence = std::unique_ptr<VkFence, FenceDeleter>;
    using UniqueVkDescriptorSetLayout = std::unique_ptr<VkDescriptorSetLayout, DescriptorSetLayoutDeleter>;
    using UniqueVkDescriptorPool = std::unique_ptr<VkDescriptorPool, DescriptorPoolDeleter>;
    using UniqueVkDescriptorSet = std::unique_ptr<VkDescriptorSet, DescriptorSetDeleter>;
    using UniqueVkBuffer = std::unique_ptr<VkBuffer, BufferDeleter>;
    using UniqueVkImage = std::unique_ptr<VkImage, ImageDeleter>;
    using UniqueVkSampler = std::unique_ptr<VkSampler, SamplerDeleter>;

    UniqueVkInstance unique(VkInstance instance);
    UniqueVkDevice unique(VkDevice device);
    UniqueVkDebugUtilsMessengerEXT unique(VkDebugUtilsMessengerEXT debug_messenger, VkInstance instance);
    UniqueVkSurfaceKHR unique(VkSurfaceKHR surface, VkInstance instance);
    UniqueVkSwapchainKHR unique(VkSwapchainKHR swapchain, VkDevice device);
    UniqueVkImageView unique(VkImageView image_view, VkDevice device);
    UniqueVkCommandPool unique(VkCommandPool command_pool, VkDevice device);
    UniqueVkCommandBuffer unique(VkCommandBuffer command_buffer, VkDevice device, VkCommandPool command_pool);
    UniqueVkRenderPass unique(VkRenderPass render_pass, VkDevice device);
    UniqueVkFramebuffer unique(VkFramebuffer framebuffer, VkDevice device);
    UniqueVkShaderModule unique(VkShaderModule shader_module, VkDevice device);
    UniqueVkPipelineLayout unique(VkPipelineLayout pipeline_layout, VkDevice device);
    UniqueVkPipeline unique(VkPipeline pipeline, VkDevice device);
    UniqueVmaAllocator unique(VmaAllocator vma_allocator);
    UniqueVkSemaphore unique(VkSemaphore semaphore, VkDevice device);
    UniqueVkFence unique(VkFence fence, VkDevice device);
    UniqueVkDescriptorSetLayout unique(VkDescriptorSetLayout descriptor_set_layout, VkDevice device);
    UniqueVkDescriptorPool unique(VkDescriptorPool descriptor_pool, VkDevice device, VkDescriptorPoolCreateFlags flags);
    UniqueVkDescriptorSet unique(VkDescriptorSet descriptor_set, VkDevice device, VkDescriptorPool descriptor_pool, bool free_descriptor_set);
    UniqueVkBuffer unique(VkBuffer buffer, VmaAllocator allocator, VmaAllocation allocation);
    UniqueVkImage unique(VkImage image, VmaAllocator allocator, VmaAllocation allocation);
    UniqueVkSampler unique(VkSampler sampler, VkDevice device);
} // namespace orion::vulkan
