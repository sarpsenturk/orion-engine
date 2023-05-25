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
        std::vector<VkLayerProperties> get_supported_layers()
        {
            std::vector<VkLayerProperties> layers;
            std::uint32_t count = 0;
            vk_result_check(vkEnumerateInstanceLayerProperties(&count, nullptr));
            layers.resize(count);
            vk_result_check(vkEnumerateInstanceLayerProperties(&count, layers.data()));
            return layers;
        }

        std::vector<VkExtensionProperties> get_supported_extensions()
        {
            std::vector<VkExtensionProperties> extensions;
            std::uint32_t count = 0;
            vk_result_check(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
            extensions.resize(count);
            vk_result_check(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()));
            return extensions;
        }

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

    UniqueVkInstance create_vk_instance(std::span<const char* const> enabled_layers, std::span<const char* const> enabled_extensions)
    {
        const auto vulkan_version = to_vulkan_version(current_version);
        const VkApplicationInfo application_info{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "OrionEngineApp",
            .applicationVersion = vulkan_version,
            .pEngineName = "OrionEngine",
            .engineVersion = vulkan_version,
            .apiVersion = vulkan_api_version,
        };

        // Check if all requested layers are supported
        {
            const auto supported_layers = get_supported_layers();
            for (const char* layer : enabled_layers) {
                const auto pred = [layer](const VkLayerProperties& layer_properties) { return std::strcmp(layer, layer_properties.layerName) == 0; };
                const auto supported = std::ranges::find_if(supported_layers, pred) != supported_layers.end();
                if (!supported) {
                    vk_result_check(VK_ERROR_LAYER_NOT_PRESENT);
                }
            }
        }

        // Check if all requested extensions are supported
        {
            const auto supported_extensions = get_supported_extensions();
            if (!check_extensions_supported(enabled_extensions, supported_extensions)) {
                vk_result_check(VK_ERROR_EXTENSION_NOT_PRESENT);
            }
        }

        // Create vulkan instance
        const VkInstanceCreateInfo instance_info{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .pApplicationInfo = &application_info,
            .enabledLayerCount = static_cast<std::uint32_t>(enabled_layers.size()),
            .ppEnabledLayerNames = enabled_layers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
            .ppEnabledExtensionNames = enabled_extensions.data(),
        };

        VkInstance instance = VK_NULL_HANDLE;
        vk_result_check(vkCreateInstance(&instance_info, alloc_callbacks(), &instance));
        return {instance, {}};
    }

    UniqueVkDebugUtilsMessengerEXT create_vk_debug_utils_messenger(VkInstance instance,
                                                                   VkDebugUtilsMessageSeverityFlagsEXT message_severity,
                                                                   VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                                   PFN_vkDebugUtilsMessengerCallbackEXT user_callback,
                                                                   void* user_data)
    {
        // Get creation function pointer
        static const auto pfn_vkCreateDebugUtilsMessengerEXT = [instance]() {
            return reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
        }();

        const VkDebugUtilsMessengerCreateInfoEXT info{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = 0,
            .messageSeverity = message_severity,
            .messageType = message_type,
            .pfnUserCallback = user_callback,
            .pUserData = user_data,
        };
        VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
        vk_result_check(pfn_vkCreateDebugUtilsMessengerEXT(instance, &info, alloc_callbacks(), &debug_messenger));
        return {debug_messenger, DebugUtilsMessengerDeleter{instance}};
    }

    UniqueVkDevice create_vk_device(VkPhysicalDevice physical_device, std::span<const std::uint32_t> queues, std::span<const char* const> enabled_extensions)
    {
        // Check if all required extensions are supported
        {
            const auto supported_extensions = get_supported_device_extensions(physical_device);
            if (!check_extensions_supported(enabled_extensions, supported_extensions)) {
                vk_result_check(VK_ERROR_EXTENSION_NOT_PRESENT);
            }
        }

        // Must declare here to avoid dangling pointers
        std::vector<float> queue_priorities(queues.size(), 1.f);

        // Create the queue create info structures
        const auto queue_infos = [queues, queue_priorities = queue_priorities.data()]() {
            std::vector<VkDeviceQueueCreateInfo> queue_infos;
            queue_infos.reserve(queues.size());
            for (auto family_index : queues) {
                queue_infos.push_back({
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .queueFamilyIndex = family_index,
                    .queueCount = 1,
                    .pQueuePriorities = queue_priorities,
                });
            }
            return queue_infos;
        }();

        // Create the device
        const VkDeviceCreateInfo device_info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = static_cast<std::uint32_t>(queue_infos.size()),
            .pQueueCreateInfos = queue_infos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
            .ppEnabledExtensionNames = enabled_extensions.data(),
            .pEnabledFeatures = nullptr,
        };
        VkDevice device = VK_NULL_HANDLE;
        vk_result_check(vkCreateDevice(physical_device, &device_info, alloc_callbacks(), &device));
        return {device, {}};
    }

    UniqueVmaAllocator create_vma_allocator(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device)
    {
        const VmaAllocatorCreateInfo allocator_info{
            .flags = 0,
            .physicalDevice = physical_device,
            .device = device,
            .preferredLargeHeapBlockSize = 0,
            .pHeapSizeLimit = nullptr,
            .pVulkanFunctions = nullptr,
            .instance = instance,
            .vulkanApiVersion = vulkan_api_version,
            .pTypeExternalMemoryHandleTypes = nullptr,
        };
        VmaAllocator allocator = VK_NULL_HANDLE;
        vk_result_check(vmaCreateAllocator(&allocator_info, &allocator));
        return {allocator, {}};
    }

    UniqueVkImageView create_vk_image_view(VkDevice device, VkImage image, VkImageViewType view_type, VkFormat format)
    {
        const VkImageViewCreateInfo image_view_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = image,
            .viewType = view_type,
            .format = format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        VkImageView image_view = VK_NULL_HANDLE;
        vk_result_check(vkCreateImageView(device, &image_view_info, alloc_callbacks(), &image_view));
        return {image_view, ImageViewDeleter{device}};
    }

    UniqueVkFramebuffer create_vk_framebuffer(VkDevice device, VkRenderPass render_pass, const math::Vector2_u& image_size, std::span<const VkImageView> attachments)
    {
        const VkFramebufferCreateInfo framebuffer_info{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = render_pass,
            .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .width = image_size.x(),
            .height = image_size.y(),
            .layers = 1,
        };
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        vk_result_check(vkCreateFramebuffer(device, &framebuffer_info, alloc_callbacks(), &framebuffer));
        return {framebuffer, FramebufferDeleter{device}};
    }

    UniqueVkShaderModule create_vk_shader_module(VkDevice device, std::span<const std::uint32_t> spirv)
    {
        const VkShaderModuleCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .codeSize = spirv.size_bytes(),
            .pCode = spirv.data(),
        };
        VkShaderModule shader_module = VK_NULL_HANDLE;
        vk_result_check(vkCreateShaderModule(device, &info, alloc_callbacks(), &shader_module));
        return {shader_module, ShaderModuleDeleter{device}};
    }

    UniqueVkPipelineLayout create_vk_pipeline_layout(VkDevice device,
                                                     std::span<const VkDescriptorSetLayout> set_layouts,
                                                     std::span<const VkPushConstantRange> push_constants)
    {
        const VkPipelineLayoutCreateInfo layout_info{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .setLayoutCount = static_cast<std::uint32_t>(set_layouts.size()),
            .pSetLayouts = set_layouts.data(),
            .pushConstantRangeCount = static_cast<std::uint32_t>(push_constants.size()),
            .pPushConstantRanges = push_constants.data(),
        };
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        vk_result_check(vkCreatePipelineLayout(device, &layout_info, alloc_callbacks(), &pipeline_layout));
        return {pipeline_layout, PipelineLayoutDeleter{device}};
    }

    UniqueVkPipeline create_vk_graphics_pipeline(VkDevice device, const VkGraphicsPipelineCreateInfo& pipeline_info)
    {
        VkPipeline pipeline = VK_NULL_HANDLE;
        vk_result_check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, alloc_callbacks(), &pipeline));
        return {pipeline, PipelineDeleter{device}};
    }

    UniqueVkCommandPool create_vk_command_pool(VkDevice device, std::uint32_t queue_family, VkCommandPoolCreateFlags flags)
    {
        const VkCommandPoolCreateInfo info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = flags,
            .queueFamilyIndex = queue_family};
        VkCommandPool command_pool = VK_NULL_HANDLE;
        vk_result_check(vkCreateCommandPool(device, &info, alloc_callbacks(), &command_pool));
        return {command_pool, CommandPoolDeleter{device}};
    }

    UniqueVkCommandBuffer allocate_vk_command_buffer(VkDevice device, VkCommandPool command_pool, VkCommandBufferLevel level)
    {
        // Allocate command buffer
        const VkCommandBufferAllocateInfo allocate_info{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = command_pool,
            .level = level,
            .commandBufferCount = 1,
        };
        VkCommandBuffer command_buffer = VK_NULL_HANDLE;
        vk_result_check(vkAllocateCommandBuffers(device, &allocate_info, &command_buffer));
        return {command_buffer, CommandBufferDeleter{device, command_pool}};
    }

    UniqueVkRenderPass create_vk_render_pass(VkDevice device,
                                             VkPipelineBindPoint bind_point,
                                             std::span<const VkAttachmentDescription> attachments,
                                             std::span<const VkAttachmentReference> color_attachments,
                                             std::span<const VkSubpassDependency> subpass_dependencies)
    {
        const VkSubpassDescription subpass{
            .flags = 0,
            .pipelineBindPoint = bind_point,
            .inputAttachmentCount = 0,
            .pInputAttachments = nullptr,
            .colorAttachmentCount = static_cast<std::uint32_t>(color_attachments.size()),
            .pColorAttachments = color_attachments.data(),
            .pResolveAttachments = nullptr,
            .pDepthStencilAttachment = nullptr,
            .preserveAttachmentCount = 0,
            .pPreserveAttachments = nullptr,
        };

        const VkRenderPassCreateInfo render_pass_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .attachmentCount = static_cast<std::uint32_t>(attachments.size()),
            .pAttachments = attachments.data(),
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = static_cast<std::uint32_t>(subpass_dependencies.size()),
            .pDependencies = subpass_dependencies.data(),
        };
        VkRenderPass render_pass = VK_NULL_HANDLE;
        vk_result_check(vkCreateRenderPass(device, &render_pass_info, alloc_callbacks(), &render_pass));
        return {render_pass, RenderPassDeleter{device}};
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
                                                             VkSwapchainKHR swapchain,
                                                             std::vector<UniqueVkFramebuffer> framebuffers,
                                                             UniqueVkSemaphore semaphore)
        : device_(device)
        , swapchain_(swapchain)
        , framebuffers_(std::move(framebuffers))
        , semaphore_(std::move(semaphore))
    {
    }

    VkFramebuffer VulkanSwapchainRenderTarget::framebuffer()
    {
        std::uint32_t available_image = 0;
        vk_result_check(vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, semaphore_.get(), VK_NULL_HANDLE, &available_image));
        return framebuffers_[available_image].get();
    }

    VkSemaphore VulkanSwapchainRenderTarget::semaphore()
    {
        return semaphore_.get();
    }
} // namespace orion::vulkan
