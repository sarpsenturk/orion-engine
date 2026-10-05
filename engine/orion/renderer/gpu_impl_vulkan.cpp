#include "gpu.hpp"

#include "orion/core/assert.hpp"
#include "orion/core/logger.hpp"
#include "orion/lib/scope_guard.hpp"

#include <Volk/volk.h>
#include <vulkan/vk_enum_string_helper.h>

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS  0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include <vma/vk_mem_alloc.h>

#include <GLFW/glfw3.h>

#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <deque>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#define GPU_CHECK(vkcall)                                                                          \
    do {                                                                                           \
        if (VkResult result = (vkcall); result != VK_SUCCESS) [[unlikely]] {                       \
            throw std::runtime_error(fmt::format(#vkcall " failed: {}", string_VkResult(result))); \
        }                                                                                          \
    } while (0)

namespace orion::gpu
{
    static constexpr auto vulkan_api_version    = VK_API_VERSION_1_3;
    static constexpr auto max_color_attachments = 8u;
    static constexpr auto max_textures          = 16u;

    static constexpr auto sampler_descriptor_binding = 0u;
    static constexpr auto texture_descriptor_binding = 1u;

    struct VulkanQueue {
        std::uint32_t family = VK_QUEUE_FAMILY_IGNORED;
        VkQueue handle       = VK_NULL_HANDLE;

        VkSemaphore semaphore = VK_NULL_HANDLE;
        std::uint64_t value   = 0ull;
    };

    struct Device {
        VkInstance instance = VK_NULL_HANDLE;

        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        VkDevice device                  = VK_NULL_HANDLE;
        VmaAllocator allocator           = VK_NULL_HANDLE;

        VulkanQueue graphics_queue = {};
        VulkanQueue compute_queue  = {};
        VulkanQueue transfer_queue = {};

        VkSampler sampler = VK_NULL_HANDLE;

        VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
        VkDescriptorPool descriptor_pool            = VK_NULL_HANDLE;
        VkDescriptorSet descriptor_set              = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout            = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties physical_device_properties = {};

        VulkanQueue& get_queue(Queue queue)
        {
            switch (queue) {
                case Queue::Graphics:
                    return graphics_queue;
                case Queue::Compute:
                    return compute_queue;
                case Queue::Transfer:
                    return transfer_queue;
            }
            std::unreachable();
        }
    };

    struct Image {
        VkImage handle           = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        std::uint32_t width  = 0u;
        std::uint32_t height = 0u;
        std::uint32_t depth  = 0u;
        VkFormat format      = VK_FORMAT_UNDEFINED;

        std::uint32_t mip_levels   = 0u;
        std::uint32_t array_layers = 0u;
    };

    struct ImageView {
        VkImageView handle = VK_NULL_HANDLE;

        std::uint32_t width  = 0u;
        std::uint32_t height = 0u;

        VkImageAspectFlags aspect_flags = 0u;
        std::uint32_t base_mip_level    = 0u;
        std::uint32_t level_count       = 1u;
        std::uint32_t base_array_layer  = 0u;
        std::uint32_t layer_count       = 1u;
    };

    struct Buffer {
        VkBuffer handle          = VK_NULL_HANDLE;
        VmaAllocation allocation = VK_NULL_HANDLE;

        void* cpu_address           = nullptr;
        VkDeviceAddress gpu_address = 0ull;
    };

    struct Swapchain {
        VkSurfaceKHR surface     = VK_NULL_HANDLE;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;

        std::vector<Image> images;
        std::vector<ImageView> image_views;

        std::vector<VkSemaphore> acquire_semaphores;
        std::vector<VkSemaphore> present_semaphores;

        std::uint32_t current_acquire_index = 0u;
        VkFormat format                     = VK_FORMAT_UNDEFINED;
    };

    struct PipelineState {
        VkPipeline pipeline              = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        VkPipelineBindPoint bind_point   = VK_PIPELINE_BIND_POINT_GRAPHICS;
        VkDescriptorSet descriptor_set   = VK_NULL_HANDLE;
    };

    struct CommandBuffer {
        VkCommandBuffer handle = VK_NULL_HANDLE;
        PipelineState* pso     = nullptr;
    };

    struct CommandAllocator {
        VkCommandPool command_pool = VK_NULL_HANDLE;
        std::deque<CommandBuffer> command_buffers;
        std::size_t current_buffer_index = 0z;
    };

    static constexpr VkFormat to_vk_format(Format format)
    {
        switch (format) {
            case Format::Undefined:
                return VK_FORMAT_UNDEFINED;
            case Format::RGBA8_Unorm:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case Format::RGBA8_Srgb:
                return VK_FORMAT_R8G8B8A8_SRGB;
            case Format::BGRA8_Unorm:
                return VK_FORMAT_B8G8R8A8_UNORM;
            case Format::BGRA8_Srgb:
                return VK_FORMAT_B8G8R8A8_SRGB;
            case Format::R32_Float:
                return VK_FORMAT_R32_SFLOAT;
            case Format::D32_Float:
                return VK_FORMAT_D32_SFLOAT;
        }
        std::unreachable();
    }

    static constexpr VkPipelineStageFlags2 to_vk_pipeline_stages(PipelineStage stages)
    {
        VkPipelineStageFlags2 output = 0ull;
        if ((stages & PipelineStage::DrawIndirect) != PipelineStage::None) {
            output |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
        }
        if ((stages & PipelineStage::IndexInput) != PipelineStage::None) {
            output |= VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT;
        }
        if ((stages & PipelineStage::VertexShader) != PipelineStage::None) {
            output |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
        }
        if ((stages & PipelineStage::FragmentShader) != PipelineStage::None) {
            output |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        }
        if ((stages & PipelineStage::DepthStencilTests) != PipelineStage::None) {
            output |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
        }
        if ((stages & PipelineStage::ColorAttachmentOutput) != PipelineStage::None) {
            output |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        if ((stages & PipelineStage::ComputeShader) != PipelineStage::None) {
            output |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        }
        if ((stages & PipelineStage::Transfer) != PipelineStage::None) {
            output |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        }
        if ((stages & PipelineStage::Host) != PipelineStage::None) {
            output |= VK_PIPELINE_STAGE_2_HOST_BIT;
        }
        if ((stages & PipelineStage::AllCommands) != PipelineStage::None) {
            output |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        }
        return output;
    }

    static VkAccessFlags2 to_vk_access(Access access)
    {
        VkAccessFlags2 output = 0ull;
        if ((access & Access::IndirectRead) != Access::None) {
            output |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
        }
        if ((access & Access::IndexRead) != Access::None) {
            output |= VK_ACCESS_2_INDEX_READ_BIT;
        }
        if ((access & Access::ShaderRead) != Access::None) {
            output |= VK_ACCESS_2_SHADER_READ_BIT;
        }
        if ((access & Access::ShaderWrite) != Access::None) {
            output |= VK_ACCESS_2_SHADER_WRITE_BIT;
        }
        if ((access & Access::ColorAttachmentRead) != Access::None) {
            output |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
        }
        if ((access & Access::ColorAttachmentWrite) != Access::None) {
            output |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        }
        if ((access & Access::DepthStencilAttachmentRead) != Access::None) {
            output |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
        }
        if ((access & Access::DepthStencilAttachmentWrite) != Access::None) {
            output |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        }
        if ((access & Access::TransferRead) != Access::None) {
            output |= VK_ACCESS_2_TRANSFER_READ_BIT;
        }
        if ((access & Access::TransferWrite) != Access::None) {
            output |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
        }
        if ((access & Access::HostRead) != Access::None) {
            output |= VK_ACCESS_2_HOST_READ_BIT;
        }
        if ((access & Access::HostWrite) != Access::None) {
            output |= VK_ACCESS_2_HOST_WRITE_BIT;
        }
        return output;
    }

    static VkImageLayout to_vk_image_layout(ImageLayout layout)
    {
        switch (layout) {
            case ImageLayout::Undefined:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            case ImageLayout::ColorAttachment:
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            case ImageLayout::DepthStencilAttachment:
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            case ImageLayout::DepthStencilReadOnly:
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            case ImageLayout::ShaderReadOnly:
                return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            case ImageLayout::TransferSrc:
                return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            case ImageLayout::TransferDst:
                return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            case ImageLayout::Present:
                return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }
        std::unreachable();
    }

    static VkPrimitiveTopology to_vk_primitive_topology(Topology topology)
    {
        switch (topology) {
            case Topology::TriangleList:
                return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
        std::unreachable();
    }

    static VkCullModeFlags to_vk_cull_mode(CullMode cull_mode)
    {
        switch (cull_mode) {
            case CullMode::None:
                return VK_CULL_MODE_NONE;
            case CullMode::Front:
                return VK_CULL_MODE_FRONT_BIT;
            case CullMode::Back:
                return VK_CULL_MODE_BACK_BIT;
        }
        std::unreachable();
    }

    static VkAttachmentLoadOp to_vk_load_op(LoadOp load_op)
    {
        switch (load_op) {
            case LoadOp::Load:
                return VK_ATTACHMENT_LOAD_OP_LOAD;
            case LoadOp::Clear:
                return VK_ATTACHMENT_LOAD_OP_CLEAR;
            case LoadOp::DontCare:
                return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        }
        std::unreachable();
    }

    static VmaAllocationCreateFlags to_vma_allocation_flags(Memory memory)
    {
        switch (memory) {
            case Memory::Default:
                return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                       VMA_ALLOCATION_CREATE_MAPPED_BIT |
                       VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT;
            case Memory::Upload:
                return VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                       VMA_ALLOCATION_CREATE_MAPPED_BIT;
            case Memory::Readback:
                return VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
                       VMA_ALLOCATION_CREATE_MAPPED_BIT;
        }
        std::unreachable();
    }

    static VkImageUsageFlags to_vk_image_usage(ImageUsage usage)
    {
        VkImageUsageFlags output = 0u;
        if ((usage & ImageUsage::TransferSrc) != ImageUsage::None) {
            output |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        }
        if ((usage & ImageUsage::TransferDst) != ImageUsage::None) {
            output |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        }
        if ((usage & ImageUsage::Sampled) != ImageUsage::None) {
            output |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }
        if ((usage & ImageUsage::Storage) != ImageUsage::None) {
            output |= VK_IMAGE_USAGE_STORAGE_BIT;
        }
        if ((usage & ImageUsage::ColorAttachment) != ImageUsage::None) {
            output |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        if ((usage & ImageUsage::DepthStencilAttachment) != ImageUsage::None) {
            output |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        return output;
    }

    static bool has_depth_component(VkFormat format)
    {
        switch (format) {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return true;
            default:
                return false;
        }
    }

    static bool has_stencil_component(VkFormat format)
    {
        switch (format) {
            case VK_FORMAT_S8_UINT:
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return true;
            default:
                return false;
        }
    }

    static VkImageAspectFlags format_aspect_flags(VkFormat format)
    {
        VkImageAspectFlags output = 0u;
        if (has_depth_component(format)) {
            output |= VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        if (has_stencil_component(format)) {
            output |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        if (output == 0u) {
            output |= VK_IMAGE_ASPECT_COLOR_BIT;
        }
        return output;
    }

    static VkBufferUsageFlags to_vk_buffer_usage(BufferUsage usage)
    {
        VkBufferUsageFlags output = 0u;
        if ((usage & BufferUsage::TransferSrc) != BufferUsage::None) {
            output |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        }
        if ((usage & BufferUsage::TransferDst) != BufferUsage::None) {
            output |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        }
        if ((usage & BufferUsage::IndexBuffer) != BufferUsage::None) {
            output |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
        }
        if ((usage & BufferUsage::IndirectBuffer) != BufferUsage::None) {
            output |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
        }
        if ((usage & BufferUsage::DeviceAddress) != BufferUsage::None) {
            output |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        }
        return output;
    }

    static std::uint32_t select_queue_family(
        const std::vector<VkQueueFamilyProperties>& queue_families,
        VkQueueFlags required_flags)
    {
        int best_score       = INT_MAX;
        std::uint32_t family = VK_QUEUE_FAMILY_IGNORED;
        for (std::uint32_t i = 0u; i < queue_families.size(); ++i) {
            const auto& qf = queue_families[i];
            if (qf.queueFlags & required_flags) {
                const auto score = std::popcount(qf.queueFlags & ~required_flags);
                if (score < best_score) {
                    best_score = score;
                    family     = i;
                }
            }
        }
        return family;
    }

    Device* create_device(const DeviceDesc& desc)
    {
        // Initialize Volk and create VkInstance
        GPU_CHECK(volkInitialize());
        std::uint32_t glfw_extension_count;
        const auto* glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        const auto app_info         = VkApplicationInfo{
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName   = "Orion Engine",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName        = "Orion Engine",
            .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion         = vulkan_api_version,
        };
        const auto instance_info = VkInstanceCreateInfo{
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo        = &app_info,
            .enabledExtensionCount   = glfw_extension_count,
            .ppEnabledExtensionNames = glfw_extensions,
        };
        VkInstance instance;
        GPU_CHECK(vkCreateInstance(&instance_info, nullptr, &instance));
        volkLoadInstance(instance);
        const auto instance_guard = ScopeFail{[&]() { vkDestroyInstance(instance, nullptr); }};

        // Select physical device
        std::uint32_t physical_device_count;
        GPU_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr));
        std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
        GPU_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data()));
        VkPhysicalDevice physical_device = physical_devices[desc.device_idx];
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(physical_device, &device_properties);

        // Select queue families
        std::uint32_t queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
        const auto graphics_family = select_queue_family(queue_families, VK_QUEUE_GRAPHICS_BIT);
        const auto compute_family  = select_queue_family(queue_families, VK_QUEUE_COMPUTE_BIT);
        const auto transfer_family = select_queue_family(queue_families, VK_QUEUE_TRANSFER_BIT);

        // Set up queue create infos
        const auto selected_families = std::set{graphics_family, compute_family, transfer_family};
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos(selected_families.size());
        static constexpr auto queue_priority = 1.0f;
        std::ranges::transform(selected_families, queue_create_infos.begin(), [&](std::uint32_t family) {
            return VkDeviceQueueCreateInfo{
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = family,
                .queueCount       = 1u,
                .pQueuePriorities = &queue_priority,
            };
        });

        // Enable device features
        auto vulkan_13_features = VkPhysicalDeviceVulkan13Features{
            .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .synchronization2 = VK_TRUE,
            .dynamicRendering = VK_TRUE,
        };
        auto vulkan_12_features = VkPhysicalDeviceVulkan12Features{
            .sType                                        = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext                                        = &vulkan_13_features,
            .descriptorIndexing                           = VK_TRUE,
            .shaderSampledImageArrayNonUniformIndexing    = VK_TRUE,
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
            .descriptorBindingUpdateUnusedWhilePending    = VK_TRUE,
            .descriptorBindingPartiallyBound              = VK_TRUE,
            .runtimeDescriptorArray                       = VK_TRUE,
            .scalarBlockLayout                            = VK_TRUE,
            .timelineSemaphore                            = VK_TRUE,
            .bufferDeviceAddress                          = VK_TRUE,
        };
        auto vulkan_11_features = VkPhysicalDeviceVulkan11Features{
            .sType                = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext                = &vulkan_12_features,
            .shaderDrawParameters = VK_TRUE,
        };
        const auto vulkan_10_features = VkPhysicalDeviceFeatures2{
            .sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext    = &vulkan_11_features,
            .features = {
                .samplerAnisotropy = VK_TRUE,
            },
        };

        // Create VkDevice
        const auto device_extensions = std::array{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        const auto device_info       = VkDeviceCreateInfo{
            .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext                   = &vulkan_10_features,
            .queueCreateInfoCount    = static_cast<std::uint32_t>(queue_create_infos.size()),
            .pQueueCreateInfos       = queue_create_infos.data(),
            .enabledExtensionCount   = static_cast<std::uint32_t>(device_extensions.size()),
            .ppEnabledExtensionNames = device_extensions.data(),
        };
        VkDevice device;
        GPU_CHECK(vkCreateDevice(physical_device, &device_info, nullptr, &device));
        volkLoadDevice(device);
        const auto device_guard = ScopeFail{[&]() { vkDestroyDevice(device, nullptr); }};

        // Initialize Vulkan Memory Allocator (VMA)
        auto vma_functions        = VmaVulkanFunctions{};
        const auto allocator_info = VmaAllocatorCreateInfo{
            .flags            = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
            .physicalDevice   = physical_device,
            .device           = device,
            .pVulkanFunctions = &vma_functions,
            .instance         = instance,
            .vulkanApiVersion = vulkan_api_version,
        };
        VmaAllocator allocator;
        GPU_CHECK(vmaImportVulkanFunctionsFromVolk(&allocator_info, &vma_functions));
        GPU_CHECK(vmaCreateAllocator(&allocator_info, &allocator));
        const auto vma_guard = ScopeFail{[&]() { vmaDestroyAllocator(allocator); }};

        // Retrieve queue handles
        VkQueue graphics_queue;
        vkGetDeviceQueue(device, graphics_family, 0u, &graphics_queue);
        VkQueue compute_queue;
        vkGetDeviceQueue(device, compute_family, 0u, &compute_queue);
        VkQueue transfer_queue;
        vkGetDeviceQueue(device, transfer_family, 0u, &transfer_queue);

        // Create queue timeline semaphores
        const auto semaphore_type_info = VkSemaphoreTypeCreateInfo{
            .sType         = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue  = 0ull,
        };
        const auto semaphore_info = VkSemaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &semaphore_type_info,
        };
        VkSemaphore graphics_semaphore;
        GPU_CHECK(vkCreateSemaphore(device, &semaphore_info, nullptr, &graphics_semaphore));
        const auto graphics_semaphore_guard = ScopeFail{[&]() { vkDestroySemaphore(device, graphics_semaphore, nullptr); }};
        VkSemaphore compute_semaphore;
        GPU_CHECK(vkCreateSemaphore(device, &semaphore_info, nullptr, &compute_semaphore));
        const auto compute_semaphore_guard = ScopeFail{[&]() { vkDestroySemaphore(device, compute_semaphore, nullptr); }};
        VkSemaphore transfer_semaphore;
        GPU_CHECK(vkCreateSemaphore(device, &semaphore_info, nullptr, &transfer_semaphore));

        // Create immutable samplers
        const auto sampler_info = VkSamplerCreateInfo{
            .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter               = VK_FILTER_LINEAR,
            .minFilter               = VK_FILTER_LINEAR,
            .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR,
            .addressModeU            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeV            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .addressModeW            = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .mipLodBias              = 0.0f,
            .anisotropyEnable        = VK_TRUE,
            .maxAnisotropy           = 16.0f,
            .compareEnable           = VK_FALSE,
            .compareOp               = VK_COMPARE_OP_ALWAYS,
            .minLod                  = 0.0f,
            .maxLod                  = 1.0f,
            .borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
            .unnormalizedCoordinates = VK_FALSE,
        };
        VkSampler sampler;
        GPU_CHECK(vkCreateSampler(device, &sampler_info, nullptr, &sampler));
        const auto sampler_guard = ScopeFail{[&]() { vkDestroySampler(device, sampler, nullptr); }};

        // Create descriptor set layout
        const auto descriptor_bindings = std::array{
            VkDescriptorSetLayoutBinding{
                .binding            = sampler_descriptor_binding,
                .descriptorType     = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount    = 1u,
                .stageFlags         = VK_SHADER_STAGE_FRAGMENT_BIT,
                .pImmutableSamplers = &sampler,
            },
            VkDescriptorSetLayoutBinding{
                .binding         = texture_descriptor_binding,
                .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = max_color_attachments,
                .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
            },
        };
        const auto descriptor_set_binding_flags = std::array{
            VkDescriptorBindingFlags{},
            VkDescriptorBindingFlags{VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                                     VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT |
                                     VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT},
        };
        const auto descriptor_set_binding_flags_info = VkDescriptorSetLayoutBindingFlagsCreateInfo{
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount  = static_cast<std::uint32_t>(descriptor_set_binding_flags.size()),
            .pBindingFlags = descriptor_set_binding_flags.data(),
        };
        const auto descriptor_set_layout_info = VkDescriptorSetLayoutCreateInfo{
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext        = &descriptor_set_binding_flags_info,
            .flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = static_cast<std::uint32_t>(descriptor_bindings.size()),
            .pBindings    = descriptor_bindings.data(),
        };
        VkDescriptorSetLayout descriptor_set_layout;
        GPU_CHECK(vkCreateDescriptorSetLayout(device, &descriptor_set_layout_info, nullptr, &descriptor_set_layout));
        const auto descriptor_set_layout_guard = ScopeFail{[&]() {
            vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
        }};

        // Create descriptor pool
        const auto descriptor_pool_sizes = std::array{
            VkDescriptorPoolSize{
                .type            = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = 1u,
            },
            VkDescriptorPoolSize{
                .type            = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = max_textures,
            },
        };
        const auto descriptor_pool_info = VkDescriptorPoolCreateInfo{
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets       = 1u,
            .poolSizeCount = static_cast<std::uint32_t>(descriptor_pool_sizes.size()),
            .pPoolSizes    = descriptor_pool_sizes.data(),
        };
        VkDescriptorPool descriptor_pool;
        GPU_CHECK(vkCreateDescriptorPool(device, &descriptor_pool_info, nullptr, &descriptor_pool));
        const auto descriptor_pool_guard = ScopeFail{[&]() { vkDestroyDescriptorPool(device, descriptor_pool, nullptr); }};

        // Allocate descriptor set
        const auto descriptor_set_allocate_info = VkDescriptorSetAllocateInfo{
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool     = descriptor_pool,
            .descriptorSetCount = 1u,
            .pSetLayouts        = &descriptor_set_layout,
        };
        VkDescriptorSet descriptor_set;
        GPU_CHECK(vkAllocateDescriptorSets(device, &descriptor_set_allocate_info, &descriptor_set));

        // Create pipeline layout
        const auto push_constant_range = VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_ALL,
            .offset     = 0u,
            .size       = max_push_constants_size,
        };
        const auto pipeline_layout_info = VkPipelineLayoutCreateInfo{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount         = 1u,
            .pSetLayouts            = &descriptor_set_layout,
            .pushConstantRangeCount = 1u,
            .pPushConstantRanges    = &push_constant_range,
        };
        VkPipelineLayout pipeline_layout;
        GPU_CHECK(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout));
        const auto pipeline_layout_guard = ScopeFail{[&]() { vkDestroyPipelineLayout(device, pipeline_layout, nullptr); }};

        return new Device{
            .instance                   = instance,
            .physical_device            = physical_device,
            .device                     = device,
            .allocator                  = allocator,
            .graphics_queue             = VulkanQueue{graphics_family, graphics_queue, graphics_semaphore},
            .compute_queue              = VulkanQueue{compute_family, compute_queue, compute_semaphore},
            .transfer_queue             = VulkanQueue{transfer_family, transfer_queue, transfer_semaphore},
            .sampler                    = sampler,
            .descriptor_set_layout      = descriptor_set_layout,
            .descriptor_pool            = descriptor_pool,
            .descriptor_set             = descriptor_set,
            .pipeline_layout            = pipeline_layout,
            .physical_device_properties = device_properties,
        };
    }

    void destroy_device(Device* device)
    {
        ORION_ASSERT(device != nullptr);

        vkDestroyPipelineLayout(device->device, device->pipeline_layout, nullptr);

        vkDestroyDescriptorPool(device->device, device->descriptor_pool, nullptr);
        vkDestroyDescriptorSetLayout(device->device, device->descriptor_set_layout, nullptr);

        vkDestroySampler(device->device, device->sampler, nullptr);

        vkDestroySemaphore(device->device, device->transfer_queue.semaphore, nullptr);
        vkDestroySemaphore(device->device, device->compute_queue.semaphore, nullptr);
        vkDestroySemaphore(device->device, device->graphics_queue.semaphore, nullptr);

        vmaDestroyAllocator(device->allocator);
        vkDestroyDevice(device->device, nullptr);
        vkDestroyInstance(device->instance, nullptr);

        delete device;
    }

    static void release_swapchain_resources(Device* device, Swapchain* swapchain)
    {
        // Reset acquire index
        swapchain->current_acquire_index = 0u;

        // Release semaphores
        std::ranges::for_each(swapchain->present_semaphores, [&](VkSemaphore semaphore) {
            vkDestroySemaphore(device->device, semaphore, nullptr);
        });
        swapchain->present_semaphores.clear();
        std::ranges::for_each(swapchain->acquire_semaphores, [&](VkSemaphore semaphore) {
            vkDestroySemaphore(device->device, semaphore, nullptr);
        });
        swapchain->acquire_semaphores.clear();

        // Release image views
        std::ranges::for_each(swapchain->image_views, [&](ImageView image_view) {
            vkDestroyImageView(device->device, image_view.handle, nullptr);
        });
        swapchain->image_views.clear();

        // Release images
        swapchain->images.clear();

        // Release swapchain
        vkDestroySwapchainKHR(device->device, swapchain->swapchain, nullptr);
    }

    static void create_or_resize_swapchain(
        Device* device,
        Swapchain* swapchain,
        std::uint32_t width,
        std::uint32_t height,
        std::uint32_t min_image_count,
        VkFormat format)
    {
        VkSurfaceCapabilitiesKHR surface_capabilities;
        GPU_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            device->physical_device, swapchain->surface, &surface_capabilities));

        // Determine swapchain extent
        VkExtent2D swapchain_extent = surface_capabilities.currentExtent;
        if (swapchain_extent.width == 0xFFFFFFFF && swapchain_extent.height == 0xFFFFFFFF) {
            swapchain_extent.width  = width;
            swapchain_extent.height = height;
        }

        // Determine swapchain image count
        min_image_count = std::clamp(
            min_image_count, surface_capabilities.minImageCount, surface_capabilities.maxImageCount);

        // Create swapchain
        const auto swapchain_info = VkSwapchainCreateInfoKHR{
            .sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .surface          = swapchain->surface,
            .minImageCount    = min_image_count,
            .imageFormat      = format,
            .imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
            .imageExtent      = swapchain_extent,
            .imageArrayLayers = 1u,
            .imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform     = surface_capabilities.currentTransform,
            .compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode      = VK_PRESENT_MODE_FIFO_KHR,
            .clipped          = VK_TRUE,
            .oldSwapchain     = swapchain->swapchain,
        };
        VkSwapchainKHR new_swapchain;
        GPU_CHECK(vkCreateSwapchainKHR(device->device, &swapchain_info, nullptr, &new_swapchain));

        // Destroy old swapchain if it exists
        if (swapchain->swapchain != VK_NULL_HANDLE) {
            release_swapchain_resources(device, swapchain);
        }
        swapchain->swapchain       = new_swapchain;
        swapchain->format          = format;
        const auto swapchain_guard = ScopeFail{[&]() { release_swapchain_resources(device, swapchain); }};

        // Retrieve swapchain images
        std::uint32_t image_count;
        GPU_CHECK(vkGetSwapchainImagesKHR(device->device, swapchain->swapchain, &image_count, nullptr));
        std::vector<VkImage> images(image_count);
        GPU_CHECK(vkGetSwapchainImagesKHR(device->device, swapchain->swapchain, &image_count, images.data()));
        swapchain->images.resize(image_count);
        std::ranges::transform(images, swapchain->images.begin(), [&](VkImage image) {
            return Image{
                .handle       = image,
                .width        = swapchain_extent.width,
                .height       = swapchain_extent.height,
                .depth        = 1u,
                .format       = format,
                .mip_levels   = 1u,
                .array_layers = 1u,
            };
        });

        // Create image views
        swapchain->image_views.reserve(image_count);
        for (const auto& image : images) {
            const auto image_view_info = VkImageViewCreateInfo{
                .sType      = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image      = image,
                .viewType   = VK_IMAGE_VIEW_TYPE_2D,
                .format     = format,
                .components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = {
                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel   = 0u,
                    .levelCount     = 1u,
                    .baseArrayLayer = 0u,
                    .layerCount     = 1u,
                },
            };
            VkImageView image_view;
            GPU_CHECK(vkCreateImageView(device->device, &image_view_info, nullptr, &image_view));
            swapchain->image_views.push_back(ImageView{
                .handle         = image_view,
                .width          = swapchain_extent.width,
                .height         = swapchain_extent.height,
                .aspect_flags   = VK_IMAGE_ASPECT_COLOR_BIT,
                .base_mip_level = 0u,
                .level_count    = 1u,
            });
        }

        // Create semaphores
        const auto semaphore_info = VkSemaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        swapchain->acquire_semaphores.resize(image_count);
        std::ranges::for_each(swapchain->acquire_semaphores, [&](VkSemaphore& semaphore) {
            GPU_CHECK(vkCreateSemaphore(device->device, &semaphore_info, nullptr, &semaphore));
        });
        swapchain->present_semaphores.resize(image_count);
        std::ranges::for_each(swapchain->present_semaphores, [&](VkSemaphore& semaphore) {
            GPU_CHECK(vkCreateSemaphore(device->device, &semaphore_info, nullptr, &semaphore));
        });
    }

    Swapchain* create_swapchain(Device* device, const SwapchainDesc& desc)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(desc.native_window_handle != nullptr);
        ORION_ASSERT(desc.width > 0u);
        ORION_ASSERT(desc.height > 0u);

        // Create VkSurfaceKHR using via GLFW
        VkSurfaceKHR surface;
        GPU_CHECK(glfwCreateWindowSurface(
            device->instance, static_cast<GLFWwindow*>(desc.native_window_handle), nullptr, &surface));
        const auto surface_guard = ScopeFail{[&]() {
            vkDestroySurfaceKHR(device->instance, surface, nullptr);
        }};

        // Create new swapchain
        auto* swapchain            = new Swapchain{.surface = surface};
        const auto swapchain_guard = ScopeFail{[&]() { destroy_swapchain(device, swapchain); }};
        create_or_resize_swapchain(
            device, swapchain,
            desc.width, desc.height,
            desc.image_count,
            to_vk_format(desc.image_format));

        return swapchain;
    }

    void resize_swapchain(Device* device, Swapchain* swapchain, std::uint32_t width, std::uint32_t height)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(swapchain != nullptr);
        ORION_ASSERT(width > 0u);
        ORION_ASSERT(height > 0u);

        create_or_resize_swapchain(
            device, swapchain,
            width, height,
            static_cast<std::uint32_t>(swapchain->images.size()),
            swapchain->format);
    }

    std::optional<SwapchainFrame> acquire_swapchain_frame(Device* device, Swapchain* swapchain)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(swapchain != nullptr);

        const auto acquire_index         = swapchain->current_acquire_index;
        swapchain->current_acquire_index = (swapchain->current_acquire_index + 1u) % swapchain->images.size();

        std::uint32_t image_index;
        switch (const auto result = vkAcquireNextImageKHR(
                    device->device,
                    swapchain->swapchain,
                    UINT64_MAX,
                    swapchain->acquire_semaphores[acquire_index],
                    VK_NULL_HANDLE,
                    &image_index)) {
            case VK_SUCCESS:
                return SwapchainFrame{
                    .image         = &swapchain->images[image_index],
                    .image_view    = &swapchain->image_views[image_index],
                    .image_index   = image_index,
                    .acquire_index = acquire_index,
                };
            case VK_SUBOPTIMAL_KHR:
                [[fallthrough]];
            case VK_ERROR_OUT_OF_DATE_KHR:
                return std::nullopt;
            default:
                throw std::runtime_error(fmt::format(
                    "vkAcquireNextImageKHR failed: {}", string_VkResult(result)));
        }
    }

    void present(Device* device, Swapchain* swapchain, const SwapchainFrame& swapchain_frame)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(swapchain != nullptr);

        const auto present_info = VkPresentInfoKHR{
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1u,
            .pWaitSemaphores    = &swapchain->present_semaphores[swapchain_frame.acquire_index],
            .swapchainCount     = 1u,
            .pSwapchains        = &swapchain->swapchain,
            .pImageIndices      = &swapchain_frame.image_index,
        };
        GPU_CHECK(vkQueuePresentKHR(device->get_queue(Queue::Graphics).handle, &present_info));
    }

    void destroy_swapchain(Device* device, Swapchain* swapchain)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(swapchain != nullptr);

        release_swapchain_resources(device, swapchain);
        vkDestroySurfaceKHR(device->instance, swapchain->surface, nullptr);

        delete swapchain;
    }

    CommandAllocator* create_command_allocator(Device* device, Queue queue)
    {
        ORION_ASSERT(device != nullptr);

        const auto command_pool_info = VkCommandPoolCreateInfo{
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags            = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = device->get_queue(queue).family,
        };
        VkCommandPool command_pool;
        GPU_CHECK(vkCreateCommandPool(device->device, &command_pool_info, nullptr, &command_pool));

        return new CommandAllocator{.command_pool = command_pool};
    }

    CommandBuffer* begin_command_buffer(Device* device, CommandAllocator* allocator)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(allocator != nullptr);

        // If there are no available command buffers in the allocator, allocate a new one
        if (allocator->current_buffer_index >= allocator->command_buffers.size()) {
            const auto command_buffer_allocate_info = VkCommandBufferAllocateInfo{
                .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool        = allocator->command_pool,
                .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1u,
            };
            VkCommandBuffer command_buffer;
            GPU_CHECK(vkAllocateCommandBuffers(device->device, &command_buffer_allocate_info, &command_buffer));
            allocator->command_buffers.emplace_back(command_buffer);
        }

        // Begin recording the command buffer
        auto* command_buffer                 = &allocator->command_buffers[allocator->current_buffer_index++];
        const auto command_buffer_begin_info = VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        };
        GPU_CHECK(vkBeginCommandBuffer(command_buffer->handle, &command_buffer_begin_info));
        return command_buffer;
    }

    void reset_command_allocator(Device* device, CommandAllocator* allocator)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(allocator != nullptr);

        // Reset the command pool, which implicitly frees all command buffers allocated from it
        GPU_CHECK(vkResetCommandPool(device->device, allocator->command_pool, 0u));
        allocator->current_buffer_index = 0z;
    }

    void destroy_command_allocator(Device* device, CommandAllocator* allocator)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(allocator != nullptr);

        vkDestroyCommandPool(device->device, allocator->command_pool, nullptr);

        delete allocator;
    }

    PipelineState* create_graphics_pipeline(Device* device, const GraphicsPipelineDesc& desc)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(!desc.spirv.empty());
        ORION_ASSERT(!desc.color_attachments.empty());
        ORION_ASSERT(desc.color_attachments.size() <= max_color_attachments);

        // Shaders
        const auto shader_module_info = VkShaderModuleCreateInfo{
            .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = desc.spirv.size_bytes(),
            .pCode    = desc.spirv.data(),
        };
        VkShaderModule shader_module;
        GPU_CHECK(vkCreateShaderModule(device->device, &shader_module_info, nullptr, &shader_module));
        const auto shader_module_guard = ScopeExit{[&]() { vkDestroyShaderModule(device->device, shader_module, nullptr); }};

        const auto shader_stages = std::array{
            VkPipelineShaderStageCreateInfo{
                .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage  = VK_SHADER_STAGE_VERTEX_BIT,
                .module = shader_module,
                .pName  = "vsmain",
            },
            VkPipelineShaderStageCreateInfo{
                .sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage  = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = shader_module,
                .pName  = "fsmain",
            },
        };

        // Vertex input
        const auto vertex_input_state = VkPipelineVertexInputStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        };

        // Input assembly
        const auto input_assembly_state = VkPipelineInputAssemblyStateCreateInfo{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology               = to_vk_primitive_topology(desc.topology),
            .primitiveRestartEnable = VK_FALSE,
        };

        // Tessellation
        const auto tessellation_state = VkPipelineTessellationStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
        };

        // Viewport and scissor
        const auto viewport_state = VkPipelineViewportStateCreateInfo{
            .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .scissorCount  = 1,
        };

        // Rasterization
        const auto rasterization_state = VkPipelineRasterizationStateCreateInfo{
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable        = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode             = VK_POLYGON_MODE_FILL,
            .cullMode                = to_vk_cull_mode(desc.cull_mode),
            .frontFace               = desc.front_face_ccw ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE,
            .depthBiasEnable         = VK_FALSE,
            .lineWidth               = 1.0f,
        };

        // Multisampling
        const auto multisample_state = VkPipelineMultisampleStateCreateInfo{
            .sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        };

        // Depth stencil
        const auto depth_stencil_state = VkPipelineDepthStencilStateCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        };

        // Color blending
        static constexpr auto blend_disabled = VkPipelineColorBlendAttachmentState{.colorWriteMask = 0xf};
        std::array<VkPipelineColorBlendAttachmentState, max_color_attachments> color_blend_attachments;
        std::ranges::fill_n(color_blend_attachments.begin(), desc.color_attachments.size(), blend_disabled);
        const auto color_blend_state = VkPipelineColorBlendStateCreateInfo{
            .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = static_cast<std::uint32_t>(desc.color_attachments.size()),
            .pAttachments    = color_blend_attachments.data(),
        };

        // Dynamic state
        static constexpr auto dynamic_states = std::array{
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };
        const auto dynamic_state = VkPipelineDynamicStateCreateInfo{
            .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
            .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
            .pDynamicStates    = dynamic_states.data(),
        };

        // Render pass
        std::array<VkFormat, 8> color_attachments;
        std::ranges::transform(desc.color_attachments, color_attachments.begin(), to_vk_format);
        const auto rendering_info = VkPipelineRenderingCreateInfo{
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount    = static_cast<std::uint32_t>(desc.color_attachments.size()),
            .pColorAttachmentFormats = color_attachments.data(),
        };

        const auto pipeline_info = VkGraphicsPipelineCreateInfo{
            .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext               = &rendering_info,
            .stageCount          = static_cast<std::uint32_t>(shader_stages.size()),
            .pStages             = shader_stages.data(),
            .pVertexInputState   = &vertex_input_state,
            .pInputAssemblyState = &input_assembly_state,
            .pTessellationState  = &tessellation_state,
            .pViewportState      = &viewport_state,
            .pRasterizationState = &rasterization_state,
            .pMultisampleState   = &multisample_state,
            .pDepthStencilState  = &depth_stencil_state,
            .pColorBlendState    = &color_blend_state,
            .pDynamicState       = &dynamic_state,
            .layout              = device->pipeline_layout,
        };
        VkPipeline pipeline;
        GPU_CHECK(vkCreateGraphicsPipelines(device->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline));

        return new PipelineState{
            .pipeline        = pipeline,
            .pipeline_layout = device->pipeline_layout,
            .bind_point      = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .descriptor_set  = device->descriptor_set,
        };
    }

    void destroy_pipeline_state(Device* device, PipelineState* pipeline_state)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(pipeline_state != nullptr);

        vkDestroyPipeline(device->device, pipeline_state->pipeline, nullptr);

        delete pipeline_state;
    }

    Image* create_image(Device* device, const ImageDesc& desc)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(desc.width > 0u);
        ORION_ASSERT(desc.height > 0u);
        ORION_ASSERT(desc.depth > 0u);
        ORION_ASSERT(desc.format != Format::Undefined);
        ORION_ASSERT(desc.mip_levels > 0u);
        ORION_ASSERT(desc.array_layers > 0u);

        const auto image_info = VkImageCreateInfo{
            .sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType     = VK_IMAGE_TYPE_2D,
            .format        = to_vk_format(desc.format),
            .extent        = {desc.width, desc.height, desc.depth},
            .mipLevels     = desc.mip_levels,
            .arrayLayers   = desc.array_layers,
            .samples       = VK_SAMPLE_COUNT_1_BIT,
            .tiling        = VK_IMAGE_TILING_OPTIMAL,
            .usage         = to_vk_image_usage(desc.usage),
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        const auto allocation_create_info = VmaAllocationCreateInfo{
            .usage = VMA_MEMORY_USAGE_AUTO,
        };
        VkImage image;
        VmaAllocation allocation;
        GPU_CHECK(vmaCreateImage(device->allocator, &image_info, &allocation_create_info, &image, &allocation, nullptr));

        return new Image{
            .handle       = image,
            .allocation   = allocation,
            .width        = image_info.extent.width,
            .height       = image_info.extent.height,
            .depth        = image_info.extent.depth,
            .format       = image_info.format,
            .mip_levels   = image_info.mipLevels,
            .array_layers = image_info.arrayLayers,
        };
    }

    Extent3D get_image_extent(const Image* image)
    {
        return Extent3D{image->width, image->height, image->depth};
    }

    Extent2D get_image_extent_2d(const Image* image)
    {
        return Extent2D{image->width, image->height};
    }

    void destroy_image(Device* device, Image* image)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(image != nullptr);

        vmaDestroyImage(device->allocator, image->handle, image->allocation);

        delete image;
    }

    ImageView* create_image_view(Device* device, const ImageViewDesc& desc)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(desc.image != nullptr);
        ORION_ASSERT(desc.base_mip_level + desc.level_count <= desc.image->mip_levels);
        const auto* image = desc.image;

        const auto image_view_info = VkImageViewCreateInfo{
            .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image            = desc.image->handle,
            .viewType         = VK_IMAGE_VIEW_TYPE_2D,
            .format           = image->format,
            .subresourceRange = {
                .aspectMask     = format_aspect_flags(image->format),
                .baseMipLevel   = desc.base_mip_level,
                .levelCount     = desc.level_count,
                .baseArrayLayer = desc.base_array_layer,
                .layerCount     = desc.layer_count,
            },
        };
        VkImageView image_view;
        GPU_CHECK(vkCreateImageView(device->device, &image_view_info, nullptr, &image_view));

        const auto width  = image->width >> desc.base_mip_level;
        const auto height = image->height >> desc.base_mip_level;

        return new ImageView{
            .handle           = image_view,
            .width            = width,
            .height           = height,
            .aspect_flags     = image_view_info.subresourceRange.aspectMask,
            .base_mip_level   = image_view_info.subresourceRange.baseMipLevel,
            .level_count      = image_view_info.subresourceRange.levelCount,
            .base_array_layer = image_view_info.subresourceRange.baseArrayLayer,
            .layer_count      = image_view_info.subresourceRange.layerCount,
        };
    }

    void set_shader_resource(Device* device, ImageView* image_view, std::uint32_t index)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(image_view != nullptr);
        ORION_ASSERT(index < max_textures);

        // Update descriptor set with the image view
        const auto descriptor_image_info = VkDescriptorImageInfo{
            .imageView   = image_view->handle,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        };
        const auto write_descriptor_set = VkWriteDescriptorSet{
            .sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet          = device->descriptor_set,
            .dstBinding      = texture_descriptor_binding,
            .dstArrayElement = index,
            .descriptorCount = 1u,
            .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .pImageInfo      = &descriptor_image_info,
        };
        vkUpdateDescriptorSets(device->device, 1, &write_descriptor_set, 0, nullptr);
    }

    void destroy_image_view(Device* device, ImageView* image_view)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(image_view != nullptr);

        vkDestroyImageView(device->device, image_view->handle, nullptr);

        delete image_view;
    }

    Buffer* create_buffer(Device* device, const BufferDesc& desc)
    {
        ORION_ASSERT(device != nullptr);

        const auto buffer_info = VkBufferCreateInfo{
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = desc.size,
            .usage       = to_vk_buffer_usage(desc.usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        const auto allocation_create_info = VmaAllocationCreateInfo{
            .flags = to_vma_allocation_flags(desc.memory),
            .usage = VMA_MEMORY_USAGE_AUTO,
        };
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocation_info;
        GPU_CHECK(vmaCreateBuffer(device->allocator, &buffer_info, &allocation_create_info, &buffer, &allocation, &allocation_info));

        // If usage includes DeviceAddress, retrieve the device address of the buffer
        VkDeviceAddress device_address = 0ull;
        if (buffer_info.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            const auto device_address_info = VkBufferDeviceAddressInfo{
                .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
                .buffer = buffer,
            };
            device_address = vkGetBufferDeviceAddress(device->device, &device_address_info);
        }

        return new Buffer{
            .handle      = buffer,
            .allocation  = allocation,
            .cpu_address = allocation_info.pMappedData,
            .gpu_address = device_address,
        };
    }

    void copy_to_buffer(Device* device, Buffer* dst, std::size_t dst_offset, std::span<const std::byte> src)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(dst != nullptr);

        GPU_CHECK(vmaCopyMemoryToAllocation(device->allocator, src.data(), dst->allocation, dst_offset, src.size_bytes()));
    }

    void copy_from_buffer(Device* device, Buffer* src, std::size_t src_offset, std::span<std::byte> dst)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(src != nullptr);

        GPU_CHECK(vmaCopyAllocationToMemory(device->allocator, src->allocation, src_offset, dst.data(), dst.size_bytes()));
    }

    Address get_buffer_device_address(const Buffer* buffer)
    {
        ORION_ASSERT(buffer != nullptr);
        return buffer->gpu_address;
    }

    void destroy_buffer(Device* device, Buffer* buffer)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(buffer != nullptr);

        vmaDestroyBuffer(device->allocator, buffer->handle, buffer->allocation);

        delete buffer;
    }

    void wait_timeline(Device* device, const TimelinePoint& timeline_point)
    {
        const auto wait_info = VkSemaphoreWaitInfo{
            .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .semaphoreCount = 1u,
            .pSemaphores    = &device->get_queue(timeline_point.queue).semaphore,
            .pValues        = &timeline_point.value,
        };
        GPU_CHECK(vkWaitSemaphores(device->device, &wait_info, UINT64_MAX));
    }

    void wait_idle(Device* device)
    {
        GPU_CHECK(vkDeviceWaitIdle(device->device));
    }

    void barrier(CommandBuffer* command_buffer, const Barrier& barrier)
    {
        std::vector<VkImageMemoryBarrier2> image_barriers(barrier.image_barriers.size());
        std::ranges::transform(barrier.image_barriers, image_barriers.begin(), [](const ImageBarrier& image_barrier) {
            return VkImageMemoryBarrier2{
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask     = to_vk_pipeline_stages(image_barrier.src_stages),
                .srcAccessMask    = to_vk_access(image_barrier.src_access),
                .dstStageMask     = to_vk_pipeline_stages(image_barrier.dst_stages),
                .dstAccessMask    = to_vk_access(image_barrier.dst_access),
                .oldLayout        = to_vk_image_layout(image_barrier.old_layout),
                .newLayout        = to_vk_image_layout(image_barrier.new_layout),
                .image            = image_barrier.image->handle,
                .subresourceRange = {
                    .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel   = 0u,
                    .levelCount     = 1u,
                    .baseArrayLayer = 0u,
                    .layerCount     = 1u,
                },
            };
        });

        const auto dependency_info = VkDependencyInfo{
            .sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .imageMemoryBarrierCount = static_cast<std::uint32_t>(image_barriers.size()),
            .pImageMemoryBarriers    = image_barriers.data(),
        };
        vkCmdPipelineBarrier2(command_buffer->handle, &dependency_info);
    }

    void bind_pso(CommandBuffer* command_buffer, PipelineState* pipeline_state)
    {
        command_buffer->pso = pipeline_state;
        vkCmdBindPipeline(command_buffer->handle, pipeline_state->bind_point, pipeline_state->pipeline);
        vkCmdBindDescriptorSets(
            command_buffer->handle,
            pipeline_state->bind_point,
            pipeline_state->pipeline_layout,
            0, 1, &pipeline_state->descriptor_set,
            0, nullptr);
    }

    void bind_index_buffer(CommandBuffer* command_buffer, Buffer* index_buffer)
    {
        vkCmdBindIndexBuffer(command_buffer->handle, index_buffer->handle, 0u, VK_INDEX_TYPE_UINT32);
    }

    void set_viewport(CommandBuffer* command_buffer, const Viewport& viewport)
    {
        // Flip the Y coordinate to account for Vulkan's coordinate system
        const auto vk_viewport = VkViewport{
            .x        = viewport.x,
            .y        = viewport.y + viewport.height,
            .width    = viewport.width,
            .height   = -viewport.height,
            .minDepth = viewport.min_depth,
            .maxDepth = viewport.max_depth,
        };
        vkCmdSetViewport(command_buffer->handle, 0u, 1u, &vk_viewport);
    }

    void set_scissor(CommandBuffer* command_buffer, const Rect2D& scissor)
    {
        const auto vk_scissor = VkRect2D{
            .offset = {
                .x = scissor.offset.x,
                .y = scissor.offset.y,
            },
            .extent = {
                .width  = scissor.extent.width,
                .height = scissor.extent.height,
            },
        };
        vkCmdSetScissor(command_buffer->handle, 0u, 1u, &vk_scissor);
    }

    void begin_render_pass(CommandBuffer* command_buffer, const RenderPass& render_pass)
    {
        std::array<VkRenderingAttachmentInfo, max_color_attachments> color_attachments;
        std::ranges::transform(render_pass.color_attachments, color_attachments.begin(), [](const RenderPassAttachment& attachment) {
            return VkRenderingAttachmentInfo{
                .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView   = attachment.image_view->handle,
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp      = to_vk_load_op(attachment.load_op),
                .clearValue  = VkClearValue{.color = VkClearColorValue{0.0f, 0.0f, 0.0f, 1.0f}},
            };
        });

        const auto* area_view     = render_pass.color_attachments[0].image_view;
        const auto rendering_info = VkRenderingInfo{
            .sType      = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {
                .extent = VkExtent2D{area_view->width, area_view->height},
            },
            .layerCount           = 1u,
            .colorAttachmentCount = static_cast<std::uint32_t>(render_pass.color_attachments.size()),
            .pColorAttachments    = color_attachments.data(),
        };
        vkCmdBeginRendering(command_buffer->handle, &rendering_info);
    }

    void end_render_pass(CommandBuffer* command_buffer)
    {
        vkCmdEndRendering(command_buffer->handle);
    }

    void draw(CommandBuffer* command_buffer, const Draw& draw)
    {
        vkCmdDraw(
            command_buffer->handle,
            draw.vertex_count,
            draw.instance_count,
            draw.first_vertex,
            draw.first_instance);
    }

    void draw_indexed(CommandBuffer* command_buffer, const DrawIndexed& draw)
    {
        vkCmdDrawIndexed(
            command_buffer->handle,
            draw.index_count,
            draw.instance_count,
            draw.first_index,
            draw.vertex_offset,
            draw.first_instance);
    }

    void push_constants(CommandBuffer* command_buffer, std::uint32_t offset, std::span<const std::byte> data)
    {
        vkCmdPushConstants(
            command_buffer->handle,
            command_buffer->pso->pipeline_layout,
            VK_SHADER_STAGE_ALL,
            offset,
            static_cast<std::uint32_t>(data.size_bytes()),
            data.data());
    }

    void copy_buffer_to_image(CommandBuffer* command_buffer, Buffer* src, Image* dst, const BufferImageCopy& copy)
    {
        const auto vk_region = VkBufferImageCopy{
            .bufferOffset      = copy.buffer_offset,
            .bufferRowLength   = copy.buffer_row_length,
            .bufferImageHeight = copy.buffer_image_height,
            .imageSubresource  = {
                .aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel       = copy.image_mip_level,
                .baseArrayLayer = copy.image_base_array_layer,
                .layerCount     = copy.image_layer_count,
            },
            .imageOffset = {
                .x = copy.image_offset.x,
                .y = copy.image_offset.y,
                .z = copy.image_offset.z,
            },
            .imageExtent = {
                .width  = copy.image_extent.width,
                .height = copy.image_extent.height,
                .depth  = copy.image_extent.depth,
            },
        };
        vkCmdCopyBufferToImage(
            command_buffer->handle,
            src->handle,
            dst->handle,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &vk_region);
    }

    TimelinePoint submit(Device* device, Queue queue, const Submit& submit)
    {
        ORION_ASSERT(device != nullptr);
        auto& vulkan_queue = device->get_queue(queue);

        std::vector<VkSemaphoreSubmitInfo> wait_infos(submit.waits.size());
        std::ranges::transform(submit.waits, wait_infos.begin(), [device](const QueueWait& wait) {
            return VkSemaphoreSubmitInfo{
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = device->get_queue(wait.timeline_point.queue).semaphore,
                .value     = wait.timeline_point.value,
                .stageMask = to_vk_pipeline_stages(wait.wait_stages),
            };
        });

        std::vector<VkCommandBufferSubmitInfo> command_buffer_infos(submit.command_buffers.size());
        std::ranges::transform(submit.command_buffers, command_buffer_infos.begin(), [](const CommandBuffer* command_buffer) {
            GPU_CHECK(vkEndCommandBuffer(command_buffer->handle));
            return VkCommandBufferSubmitInfo{
                .sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
                .commandBuffer = command_buffer->handle,
            };
        });

        std::vector<VkSemaphoreSubmitInfo> signal_infos{
            VkSemaphoreSubmitInfo{
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = vulkan_queue.semaphore,
                .value     = ++vulkan_queue.value,
                .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            },
        };

        // Append swapchain semaphores if a swapchain frame is provided
        if (submit.swapchain != nullptr && submit.swapchain_frame != nullptr) {
            wait_infos.push_back(VkSemaphoreSubmitInfo{
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = submit.swapchain->acquire_semaphores[submit.swapchain_frame->acquire_index],
                .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            });
            signal_infos.push_back(VkSemaphoreSubmitInfo{
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = submit.swapchain->present_semaphores[submit.swapchain_frame->image_index],
                .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
            });
        }

        const auto submit_info = VkSubmitInfo2{
            .sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
            .waitSemaphoreInfoCount   = static_cast<std::uint32_t>(wait_infos.size()),
            .pWaitSemaphoreInfos      = wait_infos.data(),
            .commandBufferInfoCount   = static_cast<std::uint32_t>(command_buffer_infos.size()),
            .pCommandBufferInfos      = command_buffer_infos.data(),
            .signalSemaphoreInfoCount = static_cast<std::uint32_t>(signal_infos.size()),
            .pSignalSemaphoreInfos    = signal_infos.data(),
        };
        GPU_CHECK(vkQueueSubmit2(vulkan_queue.handle, 1u, &submit_info, VK_NULL_HANDLE));

        return TimelinePoint{
            .queue = queue,
            .value = vulkan_queue.value,
        };
    }

    const char* backend_name()
    {
        return "Vulkan";
    }

    const char* backend_version()
    {
        static const auto version = []() {
            const auto api_version = volkGetInstanceVersion();
            const auto major       = VK_VERSION_MAJOR(api_version);
            const auto minor       = VK_VERSION_MINOR(api_version);
            const auto patch       = VK_VERSION_PATCH(api_version);
            return fmt::format("{}.{}.{}", major, minor, patch);
        }();
        return version.c_str();
    }

    const char* device_name(const Device* device)
    {
        ORION_ASSERT(device != nullptr);
        return device->physical_device_properties.deviceName;
    }
} // namespace orion::gpu

bool ImGui_ImplOrion_Init(const ImGui_ImplOrion_InitInfo& init_info)
{
    static constexpr auto install_glfw_callbacks = true;
    if (!ImGui_ImplGlfw_InitForVulkan(static_cast<GLFWwindow*>(init_info.native_window_handle), install_glfw_callbacks)) {
        return false;
    }

    std::array<VkFormat, orion::gpu::max_color_attachments> color_attachment_formats;
    std::ranges::transform(init_info.color_attachment_formats, color_attachment_formats.begin(), orion::gpu::to_vk_format);
    const auto imgui_pipeline_info = ImGui_ImplVulkan_PipelineInfo{
        .MSAASamples                 = VK_SAMPLE_COUNT_1_BIT,
        .PipelineRenderingCreateInfo = {
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount    = static_cast<std::uint32_t>(init_info.color_attachment_formats.size()),
            .pColorAttachmentFormats = color_attachment_formats.data(),
        },
    };
    auto imgui_init_info = ImGui_ImplVulkan_InitInfo{
        .ApiVersion               = orion::gpu::vulkan_api_version,
        .Instance                 = init_info.device->instance,
        .PhysicalDevice           = init_info.device->physical_device,
        .Device                   = init_info.device->device,
        .QueueFamily              = init_info.device->graphics_queue.family,
        .Queue                    = init_info.device->graphics_queue.handle,
        .DescriptorPoolSize       = IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE,
        .MinImageCount            = init_info.image_count,
        .ImageCount               = init_info.image_count,
        .PipelineInfoMain         = imgui_pipeline_info,
        .PipelineInfoForViewports = imgui_pipeline_info,
        .UseDynamicRendering      = true,
        .CheckVkResultFn          = +[](VkResult vkresult) { GPU_CHECK(vkresult); },
    };
    return ImGui_ImplVulkan_Init(&imgui_init_info);
}

void ImGui_ImplOrion_NewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
}

void ImGui_ImplOrion_RenderDrawData(ImDrawData* draw_data, orion::gpu::CommandBuffer* command_buffer)
{
    // Record dear imgui primitives into command buffer
    ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer->handle);
}

void ImGui_ImplOrion_Shutdown()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
}
