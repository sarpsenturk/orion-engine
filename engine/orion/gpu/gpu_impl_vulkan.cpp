#include "gpu.hpp"

#include "orion/core/assert.hpp"
#include "orion/utils/scope_guard.hpp"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "orion/core/logger.hpp"

#include <Volk/volk.h>
#include <vulkan/vk_enum_string_helper.h>

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
#include <utility>
#include <vector>

#define GPU_CHECK(vkcall)                                                                 \
    do {                                                                                  \
        if (VkResult result = (vkcall); result != VK_SUCCESS) [[unlikely]] {              \
            const auto msg = fmt::format(#vkcall " failed: {}", string_VkResult(result)); \
            ORION_LOG_ERROR(::orion::LogChannel::Gpu, msg);                               \
            throw std::runtime_error(msg);                                                \
        }                                                                                 \
    } while (0)

namespace orion::gpu
{
    // Target Vulkan API version
    static constexpr auto vulkan_api_version = VK_API_VERSION_1_4;

    // Minimum push constant size guaranteed by Vulkan.
    static constexpr std::uint32_t push_constant_size = 128u;

    struct VulkanQueue {
        VkQueue handle       = VK_NULL_HANDLE;
        std::uint32_t family = VK_QUEUE_FAMILY_IGNORED;

        VkSemaphore semaphore = VK_NULL_HANDLE;
        std::uint64_t value   = 0ull;
    };

    struct Image {
        VkImage handle        = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        std::uint32_t width  = 0u;
        std::uint32_t height = 0u;
        std::uint32_t depth  = 0u;
        VkFormat format      = VK_FORMAT_UNDEFINED;

        VkImageAspectFlags aspect_flags = 0u;
        std::uint32_t mip_levels        = 0u;
        std::uint32_t array_layers      = 0u;
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
        VkBuffer handle       = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;

        std::size_t size                = 0z;
        std::uint32_t memory_type_index = 0u;

        void* mapped                   = nullptr;
        VkDeviceAddress device_address = 0ull;
    };

    struct Sampler {
        VkSampler handle = VK_NULL_HANDLE;
    };

    struct SamplerHeap {
        VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
        VkDescriptorSet descriptor_set   = VK_NULL_HANDLE;
    };

    struct ResourceHeap {
        VkDescriptorPool descriptor_pool             = VK_NULL_HANDLE;
        VkDescriptorSet sampled_image_descriptor_set = VK_NULL_HANDLE;
        VkDescriptorSet storage_image_descriptor_set = VK_NULL_HANDLE;
    };

    struct PipelineState {
        VkPipeline handle              = VK_NULL_HANDLE;
        VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS;
    };

    struct CommandBuffer {
        VkCommandBuffer handle           = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    };

    struct CommandAllocator {
        VkCommandPool command_pool = VK_NULL_HANDLE;
        std::deque<CommandBuffer> command_buffers;
        std::size_t current_command_buffer_index = 0z;
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

    struct Device {
        VkInstance instance = VK_NULL_HANDLE;

        VkPhysicalDevice physical_device = VK_NULL_HANDLE;
        VkDevice device                  = VK_NULL_HANDLE;

        VkDescriptorSetLayout sampler_set_layout       = VK_NULL_HANDLE;
        VkDescriptorSetLayout sampled_image_set_layout = VK_NULL_HANDLE;
        VkDescriptorSetLayout storage_image_set_layout = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout               = VK_NULL_HANDLE;

        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceMemoryProperties memory_properties;

        VulkanQueue graphics_queue = {};
        VulkanQueue compute_queue  = {};
        VulkanQueue transfer_queue = {};

        [[nodiscard]] VulkanQueue& get_queue(Queue queue)
        {
            switch (queue) {
                case Queue::Graphics:
                    return graphics_queue;
                case Queue::Compute:
                    return compute_queue;
                case Queue::Transfer:
                    return transfer_queue;
                default:
                    std::unreachable();
            }
        }

        [[nodiscard]] std::uint32_t find_memory_type(
            std::uint32_t type_filter,
            VkMemoryPropertyFlags required,
            VkMemoryPropertyFlags preferred,
            VkMemoryPropertyFlags forbidden,
            VkDeviceSize min_size) const noexcept
        {
            std::uint32_t best_type = VK_MAX_MEMORY_TYPES;
            int best_score          = -1;

            for (std::uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i) {
                if ((type_filter & (1u << i)) == 0u) { continue; }
                const auto flags = memory_properties.memoryTypes[i].propertyFlags;
                if ((flags & required) != required) { continue; }
                if ((flags & forbidden) != 0u) { continue; }
                const auto& heap = memory_properties.memoryHeaps[memory_properties.memoryTypes[i].heapIndex];
                if (heap.size < min_size) { continue; }
                const auto score = std::popcount(flags & preferred);
                if (score > best_score) {
                    best_score = score;
                    best_type  = i;
                }
            }
            return best_type;
        }
    };

    static VkFormat to_vk_format(Format format)
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
            case Format::D32_Float:
                return VK_FORMAT_D32_SFLOAT;
            default:
                std::unreachable();
        }
    }

    static VkPipelineStageFlags2 to_vk_pipeline_stages(PipelineStage stages)
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

    static VkAccessFlags2 to_vk_access_flags(Access access)
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

    static VkImageUsageFlags to_vk_image_usage_flags(ImageUsage usage)
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

    static VkImageAspectFlags aspect_flags_from_format(VkFormat format)
    {
        switch (format) {
            case VK_FORMAT_D16_UNORM:
            case VK_FORMAT_X8_D24_UNORM_PACK32:
            case VK_FORMAT_D32_SFLOAT:
                return VK_IMAGE_ASPECT_DEPTH_BIT;
            case VK_FORMAT_S8_UINT:
                return VK_IMAGE_ASPECT_STENCIL_BIT;
            case VK_FORMAT_D16_UNORM_S8_UINT:
            case VK_FORMAT_D24_UNORM_S8_UINT:
            case VK_FORMAT_D32_SFLOAT_S8_UINT:
                return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            default:
                return VK_IMAGE_ASPECT_COLOR_BIT;
        }
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

    static VkAttachmentStoreOp to_vk_store_op(StoreOp store_op)
    {
        switch (store_op) {
            case StoreOp::Store:
                return VK_ATTACHMENT_STORE_OP_STORE;
            case StoreOp::DontCare:
                return VK_ATTACHMENT_STORE_OP_DONT_CARE;
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

    static VkCompareOp to_vk_compare_op(CompareOp compare_op)
    {
        switch (compare_op) {
            case CompareOp::Never:
                return VK_COMPARE_OP_NEVER;
            case CompareOp::Less:
                return VK_COMPARE_OP_LESS;
            case CompareOp::Equal:
                return VK_COMPARE_OP_EQUAL;
            case CompareOp::LessOrEqual:
                return VK_COMPARE_OP_LESS_OR_EQUAL;
            case CompareOp::Greater:
                return VK_COMPARE_OP_GREATER;
            case CompareOp::NotEqual:
                return VK_COMPARE_OP_NOT_EQUAL;
            case CompareOp::GreaterOrEqual:
                return VK_COMPARE_OP_GREATER_OR_EQUAL;
            case CompareOp::Always:
                return VK_COMPARE_OP_ALWAYS;
        }
        std::unreachable();
    }

    static VkFilter to_vk_filter(Filter filter)
    {
        switch (filter) {
            case Filter::Nearest:
                return VK_FILTER_NEAREST;
            case Filter::Linear:
                return VK_FILTER_LINEAR;
        }
        std::unreachable();
    }

    static VkSamplerAddressMode to_vk_sampler_address_mode(SamplerAddressMode address_mode)
    {
        switch (address_mode) {
            case SamplerAddressMode::Repeat:
                return VK_SAMPLER_ADDRESS_MODE_REPEAT;
            case SamplerAddressMode::MirroredRepeat:
                return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
            case SamplerAddressMode::ClampToEdge:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            case SamplerAddressMode::ClampToBorder:
                return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        }
        std::unreachable();
    }

    static VkBorderColor to_vk_border_color(BorderColor border_color)
    {
        switch (border_color) {
            case BorderColor::FloatTransparentBlack:
                return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
            case BorderColor::IntTransparentBlack:
                return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
            case BorderColor::FloatOpaqueBlack:
                return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
            case BorderColor::IntOpaqueBlack:
                return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            case BorderColor::FloatOpaqueWhite:
                return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            case BorderColor::IntOpaqueWhite:
                return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
        }
        std::unreachable();
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

    static VkMemoryPropertyFlags required_memory_properties(Memory memory)
    {
        switch (memory) {
            case Memory::GpuOnly:
                return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            case Memory::CpuVisible:
            case Memory::Readback:
                return VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
            default:
                std::unreachable();
        }
    }

    static VkMemoryPropertyFlags preferred_memory_properties(Memory memory)
    {
        switch (memory) {
            case Memory::GpuOnly:
            case Memory::CpuVisible:
                return 0u;
            case Memory::Readback:
                return VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
            default:
                std::unreachable();
        }
    }

    // The deviceCoherentMemory feature is not enabled on the device.
    static constexpr VkMemoryPropertyFlags forbidden_memory_properties = VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD;

    Device* create_device(const DeviceDesc& desc)
    {
        // Initialize Volk & create VkInstance
        ORION_LOG_TRACE(LogChannel::Gpu, "Initializing Vulkan backend...");
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
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkInstance {}", fmt::ptr(instance));
        const auto instance_guard = ScopeFail([instance] { vkDestroyInstance(instance, nullptr); });

        // Select physical device
        std::uint32_t physical_device_count;
        GPU_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr));
        std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
        GPU_CHECK(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices.data()));
        VkPhysicalDevice physical_device = physical_devices[desc.device_index];

        // Retrieve physical device & memory properties
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(physical_device, &device_properties);
        ORION_LOG_TRACE(LogChannel::Gpu, "Using VkPhysicalDevice {}", device_properties.deviceName);
        VkPhysicalDeviceMemoryProperties memory_properties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

        // Select queue families
        std::uint32_t queue_family_count;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
        const auto graphics_family = select_queue_family(queue_families, VK_QUEUE_GRAPHICS_BIT);
        ORION_LOG_TRACE(LogChannel::Gpu, "Using queue family {} for graphics", graphics_family);
        const auto compute_family = select_queue_family(queue_families, VK_QUEUE_COMPUTE_BIT);
        ORION_LOG_TRACE(LogChannel::Gpu, "Using queue family {} for compute", compute_family);
        const auto transfer_family = select_queue_family(queue_families, VK_QUEUE_TRANSFER_BIT);
        ORION_LOG_TRACE(LogChannel::Gpu, "Using queue family {} for transfer", transfer_family);

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

        // Enabled device features
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
            .shaderStorageImageArrayNonUniformIndexing    = VK_TRUE,
            .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
            .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
            .descriptorBindingUpdateUnusedWhilePending    = VK_TRUE,
            .descriptorBindingPartiallyBound              = VK_TRUE,
            .descriptorBindingVariableDescriptorCount     = VK_TRUE,
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
        const auto device_extensions = std::array{
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
        const auto device_info = VkDeviceCreateInfo{
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
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkDevice {}", fmt::ptr(device));
        const auto device_guard = ScopeFail{[device] { vkDestroyDevice(device, nullptr); }};

        // Create descriptor set layouts for samplers, sampled images, and storage images
        const auto binding_flags = VkDescriptorBindingFlags{
            VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT,
        };
        const auto binding_flags_info = VkDescriptorSetLayoutBindingFlagsCreateInfo{
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
            .bindingCount  = 1u,
            .pBindingFlags = &binding_flags,
        };

        const auto sampler_set_layout_binding = VkDescriptorSetLayoutBinding{
            .binding         = 0u,
            .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = 4'096u,
            .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
        };
        const auto sampler_set_layout_info = VkDescriptorSetLayoutCreateInfo{
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext        = &binding_flags_info,
            .flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = 1u,
            .pBindings    = &sampler_set_layout_binding,
        };
        VkDescriptorSetLayout sampler_set_layout;
        GPU_CHECK(vkCreateDescriptorSetLayout(device, &sampler_set_layout_info, nullptr, &sampler_set_layout));
        const auto sampler_set_layout_guard = ScopeFail{[device, sampler_set_layout] {
            vkDestroyDescriptorSetLayout(device, sampler_set_layout, nullptr);
        }};
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkDescriptorSetLayout {} (samplers)", fmt::ptr(sampler_set_layout));

        const auto sampled_image_set_layout_binding = VkDescriptorSetLayoutBinding{
            .binding         = 0u,
            .descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = 4'096u,
            .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT,
        };
        const auto sampled_image_set_layout_info = VkDescriptorSetLayoutCreateInfo{
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext        = &binding_flags_info,
            .flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = 1u,
            .pBindings    = &sampled_image_set_layout_binding,
        };
        VkDescriptorSetLayout sampled_image_set_layout;
        GPU_CHECK(vkCreateDescriptorSetLayout(device, &sampled_image_set_layout_info, nullptr, &sampled_image_set_layout));
        const auto sampled_image_set_layout_guard = ScopeFail{[device, sampled_image_set_layout] {
            vkDestroyDescriptorSetLayout(device, sampled_image_set_layout, nullptr);
        }};
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkDescriptorSetLayout {} (sampled images)", fmt::ptr(sampled_image_set_layout));

        const auto storage_image_set_layout_binding = VkDescriptorSetLayoutBinding{
            .binding         = 0u,
            .descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = 4'096u,
            .stageFlags      = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
        };
        const auto storage_image_set_layout_info = VkDescriptorSetLayoutCreateInfo{
            .sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .pNext        = &binding_flags_info,
            .flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
            .bindingCount = 1u,
            .pBindings    = &storage_image_set_layout_binding,
        };
        VkDescriptorSetLayout storage_image_set_layout;
        GPU_CHECK(vkCreateDescriptorSetLayout(device, &storage_image_set_layout_info, nullptr, &storage_image_set_layout));
        const auto storage_image_set_layout_guard = ScopeFail{[device, storage_image_set_layout] {
            vkDestroyDescriptorSetLayout(device, storage_image_set_layout, nullptr);
        }};
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkDescriptorSetLayout {} (storage images)", fmt::ptr(storage_image_set_layout));

        // Create the default pipeline layout shared by all pipelines on this device.
        const auto set_layouts = std::array{
            sampler_set_layout,
            sampled_image_set_layout,
            storage_image_set_layout,
        };
        const auto push_constant_range = VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_ALL,
            .offset     = 0u,
            .size       = push_constant_size,
        };
        const auto pipeline_layout_info = VkPipelineLayoutCreateInfo{
            .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount         = static_cast<std::uint32_t>(set_layouts.size()),
            .pSetLayouts            = set_layouts.data(),
            .pushConstantRangeCount = 1u,
            .pPushConstantRanges    = &push_constant_range,
        };
        VkPipelineLayout pipeline_layout;
        GPU_CHECK(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout));
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkPipelineLayout {}", fmt::ptr(pipeline_layout));
        const auto pipeline_layout_guard = ScopeFail{[device, pipeline_layout] {
            vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
        }};

        // Retrieve queue handles
        VkQueue graphics_queue;
        vkGetDeviceQueue(device, graphics_family, 0u, &graphics_queue);
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkQueue {} for graphics", fmt::ptr(graphics_queue));
        VkQueue compute_queue;
        vkGetDeviceQueue(device, compute_family, 0u, &compute_queue);
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkQueue {} for compute", fmt::ptr(compute_queue));
        VkQueue transfer_queue;
        vkGetDeviceQueue(device, transfer_family, 0u, &transfer_queue);
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkQueue {} for transfer", fmt::ptr(transfer_queue));

        // Create timeline semaphores for each queue
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
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkSemaphore {} for graphics queue", fmt::ptr(graphics_semaphore));
        const auto graphics_semaphore_guard = ScopeFail{[device, graphics_semaphore] {
            vkDestroySemaphore(device, graphics_semaphore, nullptr);
        }};
        VkSemaphore compute_semaphore;
        GPU_CHECK(vkCreateSemaphore(device, &semaphore_info, nullptr, &compute_semaphore));
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkSemaphore {} for compute queue", fmt::ptr(compute_semaphore));
        const auto compute_semaphore_guard = ScopeFail{[device, compute_semaphore] {
            vkDestroySemaphore(device, compute_semaphore, nullptr);
        }};
        VkSemaphore transfer_semaphore;
        GPU_CHECK(vkCreateSemaphore(device, &semaphore_info, nullptr, &transfer_semaphore));
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkSemaphore {} for transfer queue", fmt::ptr(transfer_semaphore));
        const auto transfer_semaphore_guard = ScopeFail{[device, transfer_semaphore] {
            vkDestroySemaphore(device, transfer_semaphore, nullptr);
        }};

        ORION_LOG_TRACE(LogChannel::Gpu, "Vulkan backend initialized");
        return new Device{
            .instance                 = instance,
            .physical_device          = physical_device,
            .device                   = device,
            .sampler_set_layout       = sampler_set_layout,
            .sampled_image_set_layout = sampled_image_set_layout,
            .storage_image_set_layout = storage_image_set_layout,
            .pipeline_layout          = pipeline_layout,
            .device_properties        = device_properties,
            .memory_properties        = memory_properties,
            .graphics_queue           = {graphics_queue, graphics_family, graphics_semaphore},
            .compute_queue            = {compute_queue, compute_family, compute_semaphore},
            .transfer_queue           = {transfer_queue, transfer_family, transfer_semaphore},
        };
    }

    void destroy_device(Device* device)
    {
        ORION_ASSERT(device != nullptr);

        vkDestroySemaphore(device->device, device->transfer_queue.semaphore, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkSemaphore {} (transfer queue)", fmt::ptr(device->transfer_queue.semaphore));
        vkDestroySemaphore(device->device, device->compute_queue.semaphore, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkSemaphore {} (compute queue)", fmt::ptr(device->compute_queue.semaphore));
        vkDestroySemaphore(device->device, device->graphics_queue.semaphore, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkSemaphore {} (graphics queue)", fmt::ptr(device->graphics_queue.semaphore));

        vkDestroyPipelineLayout(device->device, device->pipeline_layout, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkPipelineLayout {}", fmt::ptr(device->pipeline_layout));
        vkDestroyDescriptorSetLayout(device->device, device->storage_image_set_layout, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkDescriptorSetLayout {} (storage images)", fmt::ptr(device->storage_image_set_layout));
        vkDestroyDescriptorSetLayout(device->device, device->sampled_image_set_layout, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkDescriptorSetLayout {} (sampled images)", fmt::ptr(device->sampled_image_set_layout));
        vkDestroyDescriptorSetLayout(device->device, device->sampler_set_layout, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkDescriptorSetLayout {} (samplers)", fmt::ptr(device->sampler_set_layout));

        vkDestroyDevice(device->device, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkDevice {}", fmt::ptr(device));
        vkDestroyInstance(device->instance, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkInstance {}", fmt::ptr(device->instance));

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
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkSwapchainKHR {}", fmt::ptr(swapchain->swapchain));
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
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkSwapchainKHR {}", fmt::ptr(new_swapchain));
        ORION_LOG_TRACE(LogChannel::Gpu, "VkSwapchainCreateInfoKHR::minImageCount = {}",
                        swapchain_info.minImageCount);
        ORION_LOG_TRACE(LogChannel::Gpu, "VkSwapchainCreateInfoKHR::imageFormat = {}",
                        string_VkFormat(swapchain_info.imageFormat));
        ORION_LOG_TRACE(LogChannel::Gpu, "VkSwapchainCreateInfoKHR::imageColorSpace = {}",
                        string_VkColorSpaceKHR(swapchain_info.imageColorSpace));
        ORION_LOG_TRACE(LogChannel::Gpu, "VkSwapchainCreateInfoKHR::imageExtent = {},{}",
                        swapchain_info.imageExtent.width, swapchain_info.imageExtent.height);
        ORION_LOG_TRACE(LogChannel::Gpu, "VkSwapchainCreateInfoKHR::imageUsage = {}",
                        string_VkImageUsageFlags(swapchain_info.imageUsage));

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
                .aspect_flags = aspect_flags_from_format(format),
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
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkSurfaceKHR {}", fmt::ptr(surface));
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
            device,
            swapchain,
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
                    .swapchain     = swapchain,
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

    void present(Device* device, const SwapchainFrame& swapchain_frame)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(swapchain_frame.swapchain != nullptr);

        const auto present_info = VkPresentInfoKHR{
            .sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1u,
            .pWaitSemaphores    = &swapchain_frame.swapchain->present_semaphores[swapchain_frame.image_index],
            .swapchainCount     = 1u,
            .pSwapchains        = &swapchain_frame.swapchain->swapchain,
            .pImageIndices      = &swapchain_frame.image_index,
        };
        GPU_CHECK(vkQueuePresentKHR(device->graphics_queue.handle, &present_info));
    }

    void destroy_swapchain(Device* device, Swapchain* swapchain)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(swapchain != nullptr);

        release_swapchain_resources(device, swapchain);
        vkDestroySurfaceKHR(device->instance, swapchain->surface, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkSurfaceKHR {}", fmt::ptr(swapchain->surface));

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
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkCommandPool {}", fmt::ptr(command_pool));
        return new CommandAllocator{.command_pool = command_pool};
    }

    CommandBuffer* begin_command_buffer(Device* device, CommandAllocator* allocator)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(allocator != nullptr);

        // If there are no available command buffers in the allocator, allocate a new one
        if (allocator->current_command_buffer_index >= allocator->command_buffers.size()) {
            const auto command_buffer_allocate_info = VkCommandBufferAllocateInfo{
                .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .commandPool        = allocator->command_pool,
                .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1u,
            };
            VkCommandBuffer command_buffer;
            GPU_CHECK(vkAllocateCommandBuffers(device->device, &command_buffer_allocate_info, &command_buffer));
            allocator->command_buffers.emplace_back(command_buffer, device->pipeline_layout);
        }
        auto* command_buffer = &allocator->command_buffers[allocator->current_command_buffer_index++];

        // Begin recording the command buffer
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

        GPU_CHECK(vkResetCommandPool(device->device, allocator->command_pool, 0u));
        allocator->current_command_buffer_index = 0z;
    }

    void destroy_command_allocator(Device* device, CommandAllocator* allocator)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(allocator != nullptr);

        vkDestroyCommandPool(device->device, allocator->command_pool, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkCommandPool {}", fmt::ptr(allocator->command_pool));

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
            .cullMode                = to_vk_cull_mode(desc.rasterizer.cull_mode),
            .frontFace               = desc.rasterizer.front_face_ccw ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE,
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
            .sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable  = desc.depth_stencil.depth_test_enable,
            .depthWriteEnable = desc.depth_stencil.depth_write_enable,
            .depthCompareOp   = to_vk_compare_op(desc.depth_stencil.depth_compare_op),
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
        std::array<VkFormat, max_color_attachments> color_attachments;
        std::ranges::transform(desc.color_attachments, color_attachments.begin(), to_vk_format);
        const auto rendering_info = VkPipelineRenderingCreateInfo{
            .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
            .colorAttachmentCount    = static_cast<std::uint32_t>(desc.color_attachments.size()),
            .pColorAttachmentFormats = color_attachments.data(),
            .depthAttachmentFormat   = to_vk_format(desc.depth_attachment),
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
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkPipeline {} (graphics)", fmt::ptr(pipeline));

        return new PipelineState{
            .handle     = pipeline,
            .bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
        };
    }

    void destroy_pipeline(Device* device, PipelineState* pipeline)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(pipeline != nullptr);

        vkDestroyPipeline(device->device, pipeline->handle, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkPipeline {}", fmt::ptr(pipeline->handle));

        delete pipeline;
    }

    Image* create_image(Device* device, const ImageDesc& desc)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(desc.width != 0u);
        ORION_ASSERT(desc.height != 0u);
        ORION_ASSERT(desc.format != Format::Undefined);
        ORION_ASSERT(desc.mip_levels != 0u);
        ORION_ASSERT(desc.array_layers != 0u);
        ORION_ASSERT(desc.usage != ImageUsage::None);

        // Create VkImage
        const auto image_info = VkImageCreateInfo{
            .sType     = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format    = to_vk_format(desc.format),
            .extent    = {
                .width  = desc.width,
                .height = desc.height,
                .depth  = 1u,
            },
            .mipLevels     = desc.mip_levels,
            .arrayLayers   = desc.array_layers,
            .samples       = VK_SAMPLE_COUNT_1_BIT,
            .tiling        = VK_IMAGE_TILING_OPTIMAL,
            .usage         = to_vk_image_usage_flags(desc.usage),
            .sharingMode   = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        };
        VkImage image;
        GPU_CHECK(vkCreateImage(device->device, &image_info, nullptr, &image));
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkImage {}", fmt::ptr(image));
        const auto image_guard = ScopeFail{[device, image] {
            vkDestroyImage(device->device, image, nullptr);
        }};

        // Get image memory requirements
        VkMemoryRequirements memory_requirements;
        vkGetImageMemoryRequirements(device->device, image, &memory_requirements);

        // Find suitable memory type
        const auto memory_type_index = device->find_memory_type(
            memory_requirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            0u,
            VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
            memory_requirements.size);
        if (memory_type_index == VK_MAX_MEMORY_TYPES) {
            throw std::runtime_error("Vulkan: Failed to find suitable memory type for image");
        }

        // Allocate memory
        const auto allocation_info = VkMemoryAllocateInfo{
            .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .allocationSize  = memory_requirements.size,
            .memoryTypeIndex = memory_type_index,
        };
        VkDeviceMemory memory;
        GPU_CHECK(vkAllocateMemory(device->device, &allocation_info, nullptr, &memory));
        ORION_LOG_TRACE(LogChannel::Gpu, "Allocated VkDeviceMemory {} for image {}", fmt::ptr(memory), fmt::ptr(image));
        const auto memory_guard = ScopeFail{[device, memory] {
            vkFreeMemory(device->device, memory, nullptr);
        }};

        // Bind the allocation to the image
        GPU_CHECK(vkBindImageMemory(device->device, image, memory, 0));

        return new Image{
            .handle       = image,
            .memory       = memory,
            .width        = desc.width,
            .height       = desc.height,
            .depth        = 1u,
            .format       = image_info.format,
            .aspect_flags = aspect_flags_from_format(to_vk_format(desc.format)),
            .mip_levels   = desc.mip_levels,
            .array_layers = desc.array_layers,
        };
    }

    Extent2D image_extent_2d(const Image* image)
    {
        ORION_ASSERT(image != nullptr);
        return Extent2D{.width = image->width, .height = image->height};
    }

    void destroy_image(Device* device, Image* image)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(image != nullptr);
        ORION_ASSERT(image->memory != VK_NULL_HANDLE && "Can't destroy unowned images (e.g. swapchain images)");

        vkFreeMemory(device->device, image->memory, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Freed VkDeviceMemory {} for image {}", fmt::ptr(image->memory), fmt::ptr(image->handle));
        vkDestroyImage(device->device, image->handle, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkImage {}", fmt::ptr(image->handle));

        delete image;
    }

    ImageView* create_image_view(Device* device, const ImageViewDesc& desc)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(desc.image != nullptr);

        const auto image_view_info = VkImageViewCreateInfo{
            .sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image            = desc.image->handle,
            .viewType         = VK_IMAGE_VIEW_TYPE_2D,
            .format           = desc.image->format,
            .components       = {},
            .subresourceRange = {
                .aspectMask     = desc.image->aspect_flags,
                .baseMipLevel   = desc.subresource_range.base_mip_level,
                .levelCount     = desc.subresource_range.level_count,
                .baseArrayLayer = desc.subresource_range.base_array_layer,
                .layerCount     = desc.subresource_range.layer_count,
            },
        };
        VkImageView image_view;
        GPU_CHECK(vkCreateImageView(device->device, &image_view_info, nullptr, &image_view));
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkImageView {} for image {}", fmt::ptr(image_view), fmt::ptr(desc.image->handle));

        // Calculate width & height based on the base mip level
        const auto width  = std::max(1u, desc.image->width >> desc.subresource_range.base_mip_level);
        const auto height = std::max(1u, desc.image->height >> desc.subresource_range.base_mip_level);

        return new ImageView{
            .handle         = image_view,
            .width          = width,
            .height         = height,
            .aspect_flags   = desc.image->aspect_flags,
            .base_mip_level = desc.subresource_range.base_mip_level,
            .level_count    = desc.subresource_range.level_count,
        };
    }

    void destroy_image_view(Device* device, ImageView* image_view)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(image_view != nullptr);

        vkDestroyImageView(device->device, image_view->handle, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkImageView {}", fmt::ptr(image_view->handle));

        delete image_view;
    }

    Buffer* create_buffer(Device* device, const BufferDesc& desc)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(desc.size != 0z);

        // All buffers are created with the following usage flags.
        static constexpr auto default_buffer_usage =
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT |
            VK_BUFFER_USAGE_TRANSFER_DST_BIT |
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
            VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT |
            VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

        // Create VkBuffer
        const auto buffer_info = VkBufferCreateInfo{
            .sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size        = desc.size,
            .usage       = default_buffer_usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        };
        VkBuffer buffer;
        GPU_CHECK(vkCreateBuffer(device->device, &buffer_info, nullptr, &buffer));
        const auto buffer_guard = ScopeFail{[device, buffer] {
            vkDestroyBuffer(device->device, buffer, nullptr);
        }};
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkBuffer {}", fmt::ptr(buffer));

        // Get buffer memory requirements
        VkMemoryRequirements memory_requirements;
        vkGetBufferMemoryRequirements(device->device, buffer, &memory_requirements);

        // Find suitable memory type
        const auto memory_type_index = device->find_memory_type(
            memory_requirements.memoryTypeBits,
            required_memory_properties(desc.memory),
            preferred_memory_properties(desc.memory),
            forbidden_memory_properties,
            memory_requirements.size);
        if (memory_type_index == VK_MAX_MEMORY_TYPES) {
            throw std::runtime_error("Vulkan: Failed to find suitable memory type for buffer");
        }

        // Allocate memory
        const auto allocation_flags = VkMemoryAllocateFlagsInfo{
            .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO,
            .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
        };
        const auto allocation_info = VkMemoryAllocateInfo{
            .sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
            .pNext           = &allocation_flags,
            .allocationSize  = memory_requirements.size,
            .memoryTypeIndex = memory_type_index,
        };
        VkDeviceMemory memory;
        GPU_CHECK(vkAllocateMemory(device->device, &allocation_info, nullptr, &memory));
        const auto memory_guard = ScopeFail{[device, memory] {
            // Freeing mapped memory also unmaps it.
            vkFreeMemory(device->device, memory, nullptr);
        }};
        ORION_LOG_TRACE(LogChannel::Gpu, "Allocated VkDeviceMemory {} for buffer {}", fmt::ptr(memory), fmt::ptr(buffer));

        // Bind the allocation to the buffer
        GPU_CHECK(vkBindBufferMemory(device->device, buffer, memory, 0));

        // Map buffers that expose CPU access.
        void* mapped = nullptr;
        if (desc.memory != Memory::GpuOnly) {
            GPU_CHECK(vkMapMemory(device->device, memory, 0, memory_requirements.size, 0, &mapped));
        }

        // Get the device address of the buffer
        const auto buffer_address_info = VkBufferDeviceAddressInfo{
            .sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = buffer,
        };
        VkDeviceAddress device_address = vkGetBufferDeviceAddress(device->device, &buffer_address_info);

        return new Buffer{
            .handle            = buffer,
            .memory            = memory,
            .size              = desc.size,
            .memory_type_index = memory_type_index,
            .mapped            = mapped,
            .device_address    = device_address,
        };
    }

    void* buffer_host_address(const Buffer* buffer)
    {
        ORION_ASSERT(buffer != nullptr);
        return buffer->mapped;
    }

    Address buffer_device_address(const Buffer* buffer)
    {
        ORION_ASSERT(buffer != nullptr);
        return buffer->device_address;
    }

    void destroy_buffer(Device* device, Buffer* buffer)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(buffer != nullptr);

        if (buffer->mapped != nullptr) {
            vkUnmapMemory(device->device, buffer->memory);
        }

        vkFreeMemory(device->device, buffer->memory, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Freed VkDeviceMemory {} for buffer {}", fmt::ptr(buffer->memory), fmt::ptr(buffer->handle));

        vkDestroyBuffer(device->device, buffer->handle, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkBuffer {}", fmt::ptr(buffer->handle));

        delete buffer;
    }

    Sampler* create_sampler(Device* device, const SamplerDesc& desc)
    {
        ORION_ASSERT(device != nullptr);

        const auto sampler_info = VkSamplerCreateInfo{
            .sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter               = to_vk_filter(desc.filter),
            .minFilter               = to_vk_filter(desc.filter),
            .mipmapMode              = VK_SAMPLER_MIPMAP_MODE_NEAREST,
            .addressModeU            = to_vk_sampler_address_mode(desc.address_mode_u),
            .addressModeV            = to_vk_sampler_address_mode(desc.address_mode_v),
            .addressModeW            = to_vk_sampler_address_mode(desc.address_mode_w),
            .mipLodBias              = desc.mip_lod_bias,
            .anisotropyEnable        = desc.anisotropy_enable,
            .maxAnisotropy           = desc.max_anisotropy,
            .compareEnable           = desc.compare_enable,
            .compareOp               = to_vk_compare_op(desc.compare_op),
            .minLod                  = desc.min_lod,
            .maxLod                  = desc.max_lod,
            .borderColor             = to_vk_border_color(desc.border_color),
            .unnormalizedCoordinates = VK_FALSE,
        };
        VkSampler sampler = VK_NULL_HANDLE;
        GPU_CHECK(vkCreateSampler(device->device, &sampler_info, nullptr, &sampler));
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkSampler {}", fmt::ptr(sampler));

        return new Sampler{.handle = sampler};
    }

    void destroy_sampler(Device* device, Sampler* sampler)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(sampler != nullptr);

        vkDestroySampler(device->device, sampler->handle, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkSampler {}", fmt::ptr(sampler->handle));

        delete sampler;
    }

    SamplerHeap* create_sampler_heap(Device* device, const SamplerHeapDesc& desc)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(desc.sampler_count > 0u);

        // Create a descriptor pool for the samplers in the heap
        const auto pool_size = VkDescriptorPoolSize{
            .type            = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = desc.sampler_count,
        };
        const auto descriptor_pool_info = VkDescriptorPoolCreateInfo{
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets       = 1u,
            .poolSizeCount = 1u,
            .pPoolSizes    = &pool_size,
        };
        VkDescriptorPool descriptor_pool;
        GPU_CHECK(vkCreateDescriptorPool(device->device, &descriptor_pool_info, nullptr, &descriptor_pool));
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkDescriptorPool {} (samplers)", fmt::ptr(descriptor_pool));
        const auto descriptor_pool_guard = ScopeFail{[device, descriptor_pool] {
            vkDestroyDescriptorPool(device->device, descriptor_pool, nullptr);
        }};

        // Allocate the descriptor set for the samplers in the heap
        const auto variable_descriptor_count_info = VkDescriptorSetVariableDescriptorCountAllocateInfo{
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
            .descriptorSetCount = 1u,
            .pDescriptorCounts  = &desc.sampler_count,
        };
        const auto descriptor_set_allocate_info = VkDescriptorSetAllocateInfo{
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext              = &variable_descriptor_count_info,
            .descriptorPool     = descriptor_pool,
            .descriptorSetCount = 1u,
            .pSetLayouts        = &device->sampler_set_layout,
        };
        VkDescriptorSet descriptor_set;
        GPU_CHECK(vkAllocateDescriptorSets(device->device, &descriptor_set_allocate_info, &descriptor_set));
        ORION_LOG_TRACE(LogChannel::Gpu, "Allocated VkDescriptorSet {} for {} samplers", fmt::ptr(descriptor_set), desc.sampler_count);

        return new SamplerHeap{.descriptor_pool = descriptor_pool, .descriptor_set = descriptor_set};
    }

    ResourceHeap* create_resource_heap(Device* device, const ResourceHeapDesc& desc)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(desc.sampled_image_count > 0u || desc.storage_image_count > 0u);

        // Create a descriptor pool for the sampled and storage images in the heap
        const auto pool_sizes = std::array{
            VkDescriptorPoolSize{
                .type            = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = desc.sampled_image_count,
            },
            VkDescriptorPoolSize{
                .type            = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .descriptorCount = desc.storage_image_count,
            },
        };
        const auto descriptor_pool_info = VkDescriptorPoolCreateInfo{
            .sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets       = 2u,
            .poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
            .pPoolSizes    = pool_sizes.data(),
        };
        VkDescriptorPool descriptor_pool;
        GPU_CHECK(vkCreateDescriptorPool(device->device, &descriptor_pool_info, nullptr, &descriptor_pool));
        const auto descriptor_pool_guard = ScopeFail{[device, descriptor_pool] {
            vkDestroyDescriptorPool(device->device, descriptor_pool, nullptr);
        }};
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkDescriptorPool {} (sampled & storage images)", fmt::ptr(descriptor_pool));

        // Allocate the descriptor set for the sampled and storage images in the heap
        const auto variable_descriptor_sizes      = std::array{desc.sampled_image_count, desc.storage_image_count};
        const auto variable_descriptor_count_info = VkDescriptorSetVariableDescriptorCountAllocateInfo{
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
            .descriptorSetCount = 2u,
            .pDescriptorCounts  = variable_descriptor_sizes.data(),
        };
        const auto descriptor_set_layouts       = std::array{device->sampled_image_set_layout, device->storage_image_set_layout};
        const auto descriptor_set_allocate_info = VkDescriptorSetAllocateInfo{
            .sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .pNext              = &variable_descriptor_count_info,
            .descriptorPool     = descriptor_pool,
            .descriptorSetCount = 2u,
            .pSetLayouts        = descriptor_set_layouts.data(),
        };
        std::array<VkDescriptorSet, 2u> descriptor_sets;
        GPU_CHECK(vkAllocateDescriptorSets(device->device, &descriptor_set_allocate_info, descriptor_sets.data()));
        ORION_LOG_TRACE(LogChannel::Gpu,
                        "Allocated VkDescriptorSets {} and {} for {} sampled images and {} storage images",
                        fmt::ptr(descriptor_sets[0]), fmt::ptr(descriptor_sets[1]),
                        desc.sampled_image_count, desc.storage_image_count);

        return new ResourceHeap{
            .descriptor_pool              = descriptor_pool,
            .sampled_image_descriptor_set = descriptor_sets[0],
            .storage_image_descriptor_set = descriptor_sets[1],
        };
    }

    void destroy_sampler_heap(Device* device, SamplerHeap* sampler_heap)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(sampler_heap != nullptr);

        vkDestroyDescriptorPool(device->device, sampler_heap->descriptor_pool, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkDescriptorPool {} (samplers)", fmt::ptr(sampler_heap->descriptor_pool));

        delete sampler_heap;
    }

    void destroy_resource_heap(Device* device, ResourceHeap* resource_heap)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(resource_heap != nullptr);

        vkDestroyDescriptorPool(device->device, resource_heap->descriptor_pool, nullptr);
        ORION_LOG_TRACE(LogChannel::Gpu, "Destroyed VkDescriptorPool {} (sampled & storage images)", fmt::ptr(resource_heap->descriptor_pool));

        delete resource_heap;
    }

    void set_viewport(CommandBuffer* command_buffer, const Viewport& viewport)
    {
        // Flip Y coordinate to account for Vulkan's coordinate system
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

    static void push_root_data(CommandBuffer* command_buffer, ByteSpan root)
    {
        ORION_ASSERT(root.size <= push_constant_size);
        ORION_ASSERT(root.size % 4u == 0u);
        if (!root.empty()) {
            vkCmdPushConstants(
                command_buffer->handle,
                command_buffer->pipeline_layout,
                VK_SHADER_STAGE_ALL,
                0u,
                static_cast<std::uint32_t>(root.size),
                root.data);
        }
    }

    void draw(CommandBuffer* command_buffer, const Draw& draw, ByteSpan root)
    {
        push_root_data(command_buffer, root);
        vkCmdDraw(
            command_buffer->handle,
            draw.vertex_count,
            draw.instance_count,
            draw.first_vertex,
            draw.first_instance);
    }

    void draw_indexed(CommandBuffer* command_buffer, const DrawIndexed& draw, ByteSpan root)
    {
        push_root_data(command_buffer, root);
        vkCmdDrawIndexed(
            command_buffer->handle,
            draw.index_count,
            draw.instance_count,
            draw.first_index,
            draw.vertex_offset,
            draw.first_instance);
    }

    void bind_pipeline_state(CommandBuffer* command_buffer, PipelineState* pipeline_state)
    {
        vkCmdBindPipeline(command_buffer->handle, pipeline_state->bind_point, pipeline_state->handle);
    }

    void bind_index_buffer(CommandBuffer* command_buffer, Buffer* index_buffer, std::size_t offset)
    {
        vkCmdBindIndexBuffer(command_buffer->handle, index_buffer->handle, offset, VK_INDEX_TYPE_UINT32);
    }

    void begin_render_pass(CommandBuffer* command_buffer, const RenderPass& render_pass)
    {
        ORION_ASSERT(render_pass.color_attachments.size() <= max_color_attachments);

        std::array<VkRenderingAttachmentInfo, max_color_attachments> color_attachments;
        std::ranges::transform(render_pass.color_attachments, color_attachments.begin(), [](const RenderPassAttachment& attachment) {
            return VkRenderingAttachmentInfo{
                .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView   = attachment.image_view->handle,
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .loadOp      = to_vk_load_op(attachment.load_op),
                .storeOp     = to_vk_store_op(attachment.store_op),
                .clearValue  = VkClearValue{.color = {0.0f, 0.0f, 0.0f, 1.0f}},
            };
        });

        auto depth_attachment = VkRenderingAttachmentInfo{};
        if (render_pass.depth_attachment.has_value()) {
            const auto& attachment = render_pass.depth_attachment.value();
            depth_attachment       = VkRenderingAttachmentInfo{
                .sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .imageView   = attachment.image_view->handle,
                .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                .loadOp      = to_vk_load_op(attachment.load_op),
                .storeOp     = to_vk_store_op(attachment.store_op),
                .clearValue  = VkClearValue{.depthStencil = {1.0f, 0u}},
            };
        }

        const auto* area_view     = render_pass.color_attachments[0].image_view;
        const auto rendering_info = VkRenderingInfo{
            .sType      = VK_STRUCTURE_TYPE_RENDERING_INFO,
            .renderArea = {
                .offset = {},
                .extent = {area_view->width, area_view->height},
            },
            .layerCount           = 1u,
            .colorAttachmentCount = static_cast<std::uint32_t>(render_pass.color_attachments.size()),
            .pColorAttachments    = color_attachments.data(),
            .pDepthAttachment     = render_pass.depth_attachment.has_value() ? &depth_attachment : nullptr,
        };
        vkCmdBeginRendering(command_buffer->handle, &rendering_info);
    }

    void end_render_pass(CommandBuffer* command_buffer)
    {
        vkCmdEndRendering(command_buffer->handle);
    }

    void barrier(CommandBuffer* command_buffer, const Barrier& barrier)
    {
        std::vector<VkImageMemoryBarrier2> image_barriers(barrier.image_barriers.size());
        std::ranges::transform(barrier.image_barriers, image_barriers.begin(), [](const ImageBarrier& ib) {
            return VkImageMemoryBarrier2{
                .sType            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                .srcStageMask     = to_vk_pipeline_stages(ib.src_stages),
                .srcAccessMask    = to_vk_access_flags(ib.src_access),
                .dstStageMask     = to_vk_pipeline_stages(ib.dst_stages),
                .dstAccessMask    = to_vk_access_flags(ib.dst_access),
                .oldLayout        = to_vk_image_layout(ib.old_layout),
                .newLayout        = to_vk_image_layout(ib.new_layout),
                .image            = ib.image->handle,
                .subresourceRange = {
                    .aspectMask     = ib.image->aspect_flags,
                    .baseMipLevel   = ib.subresource_range.base_mip_level,
                    .levelCount     = ib.subresource_range.level_count,
                    .baseArrayLayer = ib.subresource_range.base_array_layer,
                    .layerCount     = ib.subresource_range.layer_count,
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
        if (submit.swapchain_frame != nullptr) {
            wait_infos.push_back(VkSemaphoreSubmitInfo{
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = submit.swapchain_frame->swapchain->acquire_semaphores[submit.swapchain_frame->acquire_index],
                .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            });
            signal_infos.push_back(VkSemaphoreSubmitInfo{
                .sType     = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
                .semaphore = submit.swapchain_frame->swapchain->present_semaphores[submit.swapchain_frame->image_index],
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

    void wait_idle(Device* device)
    {
        GPU_CHECK(vkDeviceWaitIdle(device->device));
    }

    void wait_queue_idle(Device* device, Queue queue)
    {
        GPU_CHECK(vkQueueWaitIdle(device->get_queue(queue).handle));
    }

    void wait_timeline(Device* device, const TimelinePoint& timeline_point)
    {
        const auto& vulkan_queue  = device->get_queue(timeline_point.queue);
        const auto semaphore_info = VkSemaphoreWaitInfo{
            .sType          = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
            .semaphoreCount = 1u,
            .pSemaphores    = &vulkan_queue.semaphore,
            .pValues        = &timeline_point.value,
        };
        GPU_CHECK(vkWaitSemaphores(device->device, &semaphore_info, UINT64_MAX));
    }

    const char* backend_name()
    {
        return "Vulkan";
    }

    const char* backend_version()
    {
        static const auto version = [] {
            const auto api_version = volkGetInstanceVersion();
            ORION_ASSERT(api_version != 0 && "create_device() must be called before getting backend version");
            const auto major = VK_VERSION_MAJOR(api_version);
            const auto minor = VK_VERSION_MINOR(api_version);
            const auto patch = VK_VERSION_PATCH(api_version);
            return fmt::format("{}.{}.{}", major, minor, patch);
        }();
        return version.c_str();
    }

    const char* device_name(const Device* device)
    {
        ORION_ASSERT(device != nullptr);
        return device->device_properties.deviceName;
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
