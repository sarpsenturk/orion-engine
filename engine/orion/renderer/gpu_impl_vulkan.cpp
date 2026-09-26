#include "gpu.hpp"

#include "orion/core/assert.hpp"
#include "orion/core/logger.hpp"
#include "orion/lib/scope_guard.hpp"

#include <Volk/volk.h>
#include <vulkan/vk_enum_string_helper.h>

#include <GLFW/glfw3.h>

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

        VulkanQueue graphics_queue = {};
        VulkanQueue compute_queue  = {};
        VulkanQueue transfer_queue = {};

        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

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
        VkImage handle  = VK_NULL_HANDLE;
        Extent3D extent = {};
    };

    struct ImageView {
        VkImageView handle = VK_NULL_HANDLE;
    };

    struct Swapchain {
        VkSurfaceKHR surface     = VK_NULL_HANDLE;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;

        std::vector<Image> images;
        std::vector<ImageView> image_views;

        std::vector<VkSemaphore> acquire_semaphores;
        std::vector<VkSemaphore> present_semaphores;

        std::uint32_t current_acquire_index = 0u;
    };

    struct PipelineState {
        VkPipeline pipeline              = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        VkPipelineBindPoint bind_point   = VK_PIPELINE_BIND_POINT_GRAPHICS;
    };

    struct CommandBuffer {
        VkCommandBuffer handle = VK_NULL_HANDLE;
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
            case Format::BGRA8_Unorm:
                return VK_FORMAT_B8G8R8A8_UNORM;
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

        // Create pipeline layout
        const auto pipeline_layout_info = VkPipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        };
        VkPipelineLayout pipeline_layout;
        GPU_CHECK(vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout));
        const auto pipeline_layout_guard = ScopeFail{[&]() { vkDestroyPipelineLayout(device, pipeline_layout, nullptr); }};

        return new Device{
            .instance                   = instance,
            .physical_device            = physical_device,
            .device                     = device,
            .graphics_queue             = VulkanQueue{graphics_family, graphics_queue, graphics_semaphore},
            .compute_queue              = VulkanQueue{compute_family, compute_queue, compute_semaphore},
            .transfer_queue             = VulkanQueue{transfer_family, transfer_queue, transfer_semaphore},
            .pipeline_layout            = pipeline_layout,
            .physical_device_properties = device_properties,
        };
    }

    void destroy_device(Device* device)
    {
        ORION_ASSERT(device != nullptr);

        vkDestroyPipelineLayout(device->device, device->pipeline_layout, nullptr);

        vkDestroySemaphore(device->device, device->transfer_queue.semaphore, nullptr);
        vkDestroySemaphore(device->device, device->compute_queue.semaphore, nullptr);
        vkDestroySemaphore(device->device, device->graphics_queue.semaphore, nullptr);

        vkDestroyDevice(device->device, nullptr);
        vkDestroyInstance(device->instance, nullptr);

        delete device;
    }

    static void release_swapchain_resources(Device* device, Swapchain* swapchain)
    {
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
        const auto swapchain_guard = ScopeFail{[&]() { release_swapchain_resources(device, swapchain); }};

        // Retrieve swapchain images
        std::uint32_t image_count;
        GPU_CHECK(vkGetSwapchainImagesKHR(device->device, swapchain->swapchain, &image_count, nullptr));
        std::vector<VkImage> images(image_count);
        GPU_CHECK(vkGetSwapchainImagesKHR(device->device, swapchain->swapchain, &image_count, images.data()));
        swapchain->images.resize(image_count);
        std::ranges::transform(images, swapchain->images.begin(), [&](VkImage image) {
            return Image{
                .handle = image,
                .extent = Extent3D{
                    .width  = swapchain_extent.width,
                    .height = swapchain_extent.height,
                    .depth  = 1u,
                },
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
            swapchain->image_views.push_back(ImageView{.handle = image_view});
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
        };
    }

    void destroy_pipeline_state(Device* device, PipelineState* pipeline_state)
    {
        ORION_ASSERT(device != nullptr);
        ORION_ASSERT(pipeline_state != nullptr);

        vkDestroyPipeline(device->device, pipeline_state->pipeline, nullptr);

        delete pipeline_state;
    }

    Extent3D get_image_extent(const Image* image)
    {
        return image->extent;
    }

    Extent2D get_image_extent_2d(const Image* image)
    {
        return Extent2D{
            .width  = image->extent.width,
            .height = image->extent.height,
        };
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
        vkCmdBindPipeline(command_buffer->handle, pipeline_state->bind_point, pipeline_state->pipeline);
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
