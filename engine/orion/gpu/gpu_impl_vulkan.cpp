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
