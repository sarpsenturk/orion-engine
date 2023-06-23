#pragma once

#include "vulkan_headers.h"

#include <orion-math/vector/vector2.h> // orion::Vector2
#include <span>                        // std::span
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

    template<typename UniqueType, typename VulkanHandle, typename... DeleterArgs>
    auto vk_unique(VulkanHandle vk_handle, DeleterArgs... deleter_args)
    {
        return UniqueType{vk_handle, {deleter_args...}};
    }

    UniqueVkInstance create_vk_instance(std::span<const char* const> enabled_layers, std::span<const char* const> enabled_extensions);
    UniqueVkDebugUtilsMessengerEXT create_vk_debug_utils_messenger(VkInstance instance,
                                                                   VkDebugUtilsMessageSeverityFlagsEXT message_severity,
                                                                   VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                                   PFN_vkDebugUtilsMessengerCallbackEXT user_callback,
                                                                   void* user_data);
    UniqueVkDevice create_vk_device(VkPhysicalDevice physical_device, std::span<const std::uint32_t> queues, std::span<const char* const> enabled_extensions);
    UniqueVmaAllocator create_vma_allocator(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device);
    UniqueVkImageView create_vk_image_view(VkDevice device, VkImage image, VkImageViewType view_type, VkFormat format);
    UniqueVkFramebuffer create_vk_framebuffer(VkDevice device, VkRenderPass render_pass, const Vector2_u& image_size, std::span<const VkImageView> attachments);
    UniqueVkShaderModule create_vk_shader_module(VkDevice device, std::span<const std::uint32_t> spirv);
    UniqueVkPipelineLayout create_vk_pipeline_layout(VkDevice device, std::span<const VkDescriptorSetLayout> set_layouts, std::span<const VkPushConstantRange> push_constants);
    UniqueVkPipeline create_vk_graphics_pipeline(VkDevice device, const VkGraphicsPipelineCreateInfo& pipeline_info);
    UniqueVkCommandPool create_vk_command_pool(VkDevice device, std::uint32_t queue_family, VkCommandPoolCreateFlags flags = 0);
    UniqueVkCommandBuffer allocate_vk_command_buffer(VkDevice device, VkCommandPool command_pool, VkCommandBufferLevel level);
    UniqueVkRenderPass create_vk_render_pass(VkDevice device,
                                             VkPipelineBindPoint bind_point,
                                             std::span<const VkAttachmentDescription> attachments,
                                             std::span<const VkAttachmentReference> color_attachments,
                                             std::span<const VkSubpassDependency> subpass_dependencies);
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

        VkFramebuffer framebuffer() const;
        VkSemaphore semaphore() const { return semaphore_.get(); }

    private:
        VkDevice device_;
        VulkanSwapchain* swapchain_;
        std::vector<UniqueVkFramebuffer> framebuffers_;
        UniqueVkSemaphore semaphore_;
    };

    using VulkanRenderTarget = std::variant<VulkanSwapchainRenderTarget>;
} // namespace orion::vulkan
