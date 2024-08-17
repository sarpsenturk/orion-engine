#include "vulkan_backend.h"

#include "vulkan_error.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <vector>

namespace orion
{
    namespace
    {
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
#ifdef ORION_BUILD_DEBUG
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
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
                .pApplicationName = "OrionVulkankApp",
                .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                .pEngineName = "OrionVulkanEngine",
                .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                .apiVersion = VK_VERSION_1_2,
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
} // namespace orion
