#pragma once

#include "vulkan_headers.h"

namespace orion::vulkan
{
    struct VulkanQueues {
        struct Queue {
            std::uint32_t index;
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

    using UniqueVkInstance = std::unique_ptr<VkInstance, InstanceDeleter>;
    using UniqueVkDevice = std::unique_ptr<VkDevice, DeviceDeleter>;
    using UniqueVkDebugUtilsMessengerEXT = std::unique_ptr<VkDebugUtilsMessengerEXT, DebugUtilsMessengerDeleter>;
    using UniqueVkSurfaceKHR = std::unique_ptr<VkSurfaceKHR, SurfaceDeleter>;
    using UniqueVkSwapchainKHR = std::unique_ptr<VkSwapchainKHR, SwapchainDeleter>;
    using UniqueVkImageView = std::unique_ptr<VkImageView, ImageViewDeleter>;
    using UniqueVkCommandPool = std::unique_ptr<VkCommandPool, CommandPoolDeleter>;
    using UniqueVkRenderPass = std::unique_ptr<VkRenderPass, RenderPassDeleter>;
    using UniqueVkFramebuffer = std::unique_ptr<VkFramebuffer, FramebufferDeleter>;
    using UniqueVkShaderModule = std::unique_ptr<VkShaderModule, ShaderModuleDeleter>;
    using UniqueVkPipelineLayout = std::unique_ptr<VkPipelineLayout, PipelineLayoutDeleter>;
    using UniqueVkPipeline = std::unique_ptr<VkPipeline, PipelineDeleter>;
    using UniqueVmaAllocator = std::unique_ptr<VmaAllocator, AllocatorDeleter>;
    using UniqueVkSemaphore = std::unique_ptr<VkSemaphore, SemaphoreDeleter>;
    using UniqueVkFence = std::unique_ptr<VkFence, FenceDeleter>;

    UniqueVkCommandPool create_command_pool(VkDevice device, std::uint32_t queue_family, VkCommandPoolCreateFlags flags = 0);
    UniqueVkSemaphore create_semaphore(VkDevice device);
    UniqueVkFence create_fence(VkDevice device, VkFenceCreateFlags flags = 0);
} // namespace orion::vulkan
