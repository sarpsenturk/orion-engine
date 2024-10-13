#include "vulkan_backend.hpp"

#include "vulkan_device.hpp"
#include "vulkan_error.hpp"

#include "orion/assertion.hpp"
#include "orion/platform.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <vector>

namespace orion
{
    namespace
    {
#ifdef ORION_PLATFORM_WIN32
        constexpr const char* platform_surface_ext = "VK_KHR_win32_surface";
#else
    #error no platform surface extension defined
#endif

        std::vector<const char*> enabled_instance_layers()
        {
            std::vector<const char*> layers;
#ifdef ORION_BUILD_DEBUG
            layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
            return layers;
        }

        std::vector<const char*> enabled_instance_extensions()
        {
            std::vector<const char*> extensions;
            extensions.push_back("VK_KHR_surface");
            extensions.push_back(platform_surface_ext);
#ifdef ORION_BUILD_DEBUG
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
            return extensions;
        }

        std::vector<const char*> enabled_device_extensions()
        {
            std::vector<const char*> extensions;
            extensions.push_back("VK_KHR_swapchain");
            extensions.push_back("VK_KHR_dynamic_rendering");
            return extensions;
        }

        VkBool32 vulkan_message_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                         VkDebugUtilsMessageTypeFlagsEXT,
                                         const VkDebugUtilsMessengerCallbackDataEXT* message,
                                         void*)
        {
            if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
                SPDLOG_ERROR("{}", message->pMessage);
            } else if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                SPDLOG_WARN("{}", message->pMessage);
            }
            return VK_FALSE;
        }
    } // namespace

    VulkanBackend::VulkanBackend()
    {
        // Initialize volk
        vk_assert(volkInitialize());

        // Create Vulkan instance
        {
            const auto app_info = VkApplicationInfo{
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pNext = nullptr,
                .pApplicationName = "OrionVulkanApp",
                .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                .pEngineName = "OrionVulkanEngine",
                .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                .apiVersion = VK_API_VERSION_1_2,
            };
            const auto layers = enabled_instance_layers();
            const auto extensions = enabled_instance_extensions();
            const auto instance_info = VkInstanceCreateInfo{
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .pApplicationInfo = &app_info,
                .enabledLayerCount = static_cast<uint32_t>(layers.size()),
                .ppEnabledLayerNames = layers.data(),
                .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
                .ppEnabledExtensionNames = extensions.data(),
            };
            vk_assert(vkCreateInstance(&instance_info, nullptr, &instance_), "Failed to create Vulkan instance");
            SPDLOG_TRACE("Created VkInstance {}", fmt::ptr(instance_));
        }

        volkLoadInstance(instance_);

// Create debug messenger
#ifdef ORION_BUILD_DEBUG
        {
            const auto messenger_info = VkDebugUtilsMessengerCreateInfoEXT{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .pNext = nullptr,
                .flags = 0,
                .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
                .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
                .pfnUserCallback = &vulkan_message_callback,
                .pUserData = nullptr,
            };
            vk_assert(vkCreateDebugUtilsMessengerEXT(instance_, &messenger_info, nullptr, &debug_messenger_), "Failed to create Vulkan debug messenger");
            SPDLOG_TRACE("Created VkDebugUtilsMessengerEXT {}", fmt::ptr(debug_messenger_));
        }
#endif

        // Get physical devices
        get_adapters_api();

        SPDLOG_DEBUG("Vulkan backend initialized");
    }

    VulkanBackend::~VulkanBackend()
    {
        if (debug_messenger_ != VK_NULL_HANDLE) {
            vkDestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
        }
        if (instance_ != VK_NULL_HANDLE) {
            vkDestroyInstance(instance_, nullptr);
        }
    }

    std::vector<GraphicsAdapter> VulkanBackend::get_adapters_api()
    {
        std::uint32_t count = 0;
        vk_assert(vkEnumeratePhysicalDevices(instance_, &count, nullptr), "Failed to enumerate physical devices");
        physical_devices_.resize(count);
        vk_assert(vkEnumeratePhysicalDevices(instance_, &count, physical_devices_.data()), "Failed to enumerate physical devices");

        std::vector<GraphicsAdapter> adapters(count);
        std::ranges::transform(physical_devices_, adapters.begin(), [](VkPhysicalDevice physical_device) {
            VkPhysicalDeviceProperties properties;
            vkGetPhysicalDeviceProperties(physical_device, &properties);
            return GraphicsAdapter{
                .name = properties.deviceName,
            };
        });
        return adapters;
    }

    std::unique_ptr<RenderDevice> VulkanBackend::create_device_api(std::size_t adapter_index)
    {
        ORION_EXPECTS(adapter_index < physical_devices_.size());
        VkPhysicalDevice physical_device = physical_devices_[adapter_index];
        SPDLOG_TRACE("Creating VkDevice with VkPhysicalDevice {}", fmt::ptr(physical_device));

        const auto queue_families = [&] {
            std::uint32_t count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
            std::vector<VkQueueFamilyProperties> properties(count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, properties.data());
            return properties;
        }();
        SPDLOG_TRACE("Found {} queue families for physical device", queue_families.size());

        const auto queue_family_index = [&] {
            for (std::uint32_t index = 0; const auto& queue_family : queue_families) {
                if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    return index;
                }
                ++index;
            }
            return UINT32_MAX;
        }();
        if (queue_family_index == UINT32_MAX) {
            throw std::runtime_error{"Vulkan: failed to find suitable queue family"};
        }
        SPDLOG_TRACE("Using queue family index {}", queue_family_index);

        const auto queue_priority = 1.f;
        const auto queue_info = VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };

        VkDevice device = VK_NULL_HANDLE;

        const auto extensions = enabled_device_extensions();
        const auto dynamic_rendering = VkPhysicalDeviceDynamicRenderingFeatures{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
            .pNext = nullptr,
            .dynamicRendering = VK_TRUE,
        };
        const auto device_info = VkDeviceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &dynamic_rendering,
            .flags = 0,
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queue_info,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<std::uint32_t>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
            .pEnabledFeatures = nullptr,
        };
        vk_assert(vkCreateDevice(physical_device, &device_info, nullptr, &device));
        SPDLOG_TRACE("Created VkDevice {}", fmt::ptr(device));

        volkLoadDevice(device);

        VkQueue queue;
        vkGetDeviceQueue(device, queue_family_index, 0, &queue);

        return std::make_unique<VulkanDevice>(device, instance_, physical_device, queue, queue_family_index);
    }
} // namespace orion
