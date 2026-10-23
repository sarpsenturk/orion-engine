#include "gpu.hpp"

#include "orion/core/assert.hpp"
#include "orion/core/logger.hpp"
#include "orion/utils/scope_guard.hpp"

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
    static constexpr auto vulkan_api_version = VK_API_VERSION_1_3;

    struct VulkanQueue {
        VkQueue handle       = VK_NULL_HANDLE;
        std::uint32_t family = VK_QUEUE_FAMILY_IGNORED;
    };

    struct Image {
        VkImage handle = VK_NULL_HANDLE;

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

    struct CommandBuffer {
        VkCommandBuffer handle = VK_NULL_HANDLE;
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
        VkPhysicalDeviceProperties device_properties;

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
            default:
                std::unreachable();
        }
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
        VkPhysicalDeviceProperties device_properties;
        vkGetPhysicalDeviceProperties(physical_device, &device_properties);
        ORION_LOG_TRACE(LogChannel::Gpu, "Using VkPhysicalDevice {}", device_properties.deviceName);

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
            .sType                                     = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext                                     = &vulkan_13_features,
            .descriptorIndexing                        = VK_TRUE,
            .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
            .scalarBlockLayout                         = VK_TRUE,
            .timelineSemaphore                         = VK_TRUE,
            .bufferDeviceAddress                       = VK_TRUE,
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
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkDevice {}", fmt::ptr(device));
        const auto device_guard = ScopeFail{[device] { vkDestroyDevice(device, nullptr); }};

        // Retrieve queue handles
        VkQueue graphics_queue;
        vkGetDeviceQueue(device, graphics_family, 0u, &graphics_queue);
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkQueue {} for graphics", fmt::ptr(graphics_queue));
        VkQueue compute_queue;
        vkGetDeviceQueue(device, compute_family, 0u, &compute_queue);
        ORION_LOG_TRACE(LogChannel::Gpu, "Created VkQueue {} for compute", fmt::ptr(compute_queue));
        VkQueue transfer_queue;
        vkGetDeviceQueue(device, transfer_family, 0u, &transfer_queue);
        ORION_LOG_DEBUG(LogChannel::Gpu, "Created VkQueue {} for transfer", fmt::ptr(transfer_queue));

        ORION_LOG_TRACE(LogChannel::Gpu, "Vulkan backend initialized");
        return new Device{
            .instance          = instance,
            .physical_device   = physical_device,
            .device            = device,
            .device_properties = device_properties,
            .graphics_queue    = {graphics_queue, graphics_family},
            .compute_queue     = {compute_queue, compute_family},
            .transfer_queue    = {transfer_queue, transfer_family},
        };
    }

    void destroy_device(Device* device)
    {
        ORION_ASSERT(device != nullptr);

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
            allocator->command_buffers.emplace_back(command_buffer);
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
