#pragma once

#include "vulkan_headers.h"

#include <orion-math/vector/vector2.h>

#include <memory>
#include <span>
#include <variant>

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

        void operator()(VkDescriptorPool descriptor_pool) const;
    };

    struct DescriptorSetDeleter {
        using pointer = VkDescriptorSet;

        VkDevice device = VK_NULL_HANDLE;
        VkDescriptorPool pool = VK_NULL_HANDLE;

        void operator()(VkDescriptorSet descriptor_set) const;
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
    UniqueVkDescriptorPool unique(VkDescriptorPool descriptor_pool, VkDevice device);
    UniqueVkDescriptorSet unique(VkDescriptorSet descriptor_set, VkDevice device, VkDescriptorPool descriptor_pool);

    UniqueVkSemaphore create_vk_semaphore(VkDevice device);
    UniqueVkFence create_vk_fence(VkDevice device, VkFenceCreateFlags flags = 0);

    class VulkanSwapchain
    {
    public:
        VulkanSwapchain(VkSurfaceKHR surface, UniqueVkSwapchainKHR swapchain, std::vector<UniqueVkImageView> image_views);

        [[nodiscard]] auto surface() const noexcept { return surface_; }
        [[nodiscard]] auto swapchain() const noexcept { return swapchain_.get(); }
        [[nodiscard]] auto& image_views() const noexcept { return image_views_; }
        [[nodiscard]] auto image_index() const noexcept { return image_index_; }

        void set_image_index(std::uint32_t index) noexcept { image_index_ = index; }

    private:
        VkSurfaceKHR surface_;
        UniqueVkSwapchainKHR swapchain_;
        std::vector<UniqueVkImageView> image_views_;
        std::uint32_t image_index_ = UINT32_MAX;
    };

    class VulkanSwapchainRenderTarget
    {
    public:
        VulkanSwapchainRenderTarget(VkDevice device,
                                    VulkanSwapchain* swapchain,
                                    std::vector<UniqueVkFramebuffer> framebuffers,
                                    UniqueVkSemaphore semaphore);

        [[nodiscard]] VkFramebuffer framebuffer() const;
        [[nodiscard]] VkSemaphore semaphore() const { return semaphore_.get(); }

    private:
        VkDevice device_;
        VulkanSwapchain* swapchain_;
        std::vector<UniqueVkFramebuffer> framebuffers_;
        UniqueVkSemaphore semaphore_;
    };

    using VulkanRenderTarget = std::variant<VulkanSwapchainRenderTarget>;

    struct VulkanSubmission {
        UniqueVkFence fence = VK_NULL_HANDLE;
        UniqueVkSemaphore semaphore = VK_NULL_HANDLE;
        std::vector<VkSemaphore> wait_semaphores;
    };
} // namespace orion::vulkan
