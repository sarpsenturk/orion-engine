#include "vulkan_impl.hpp"

#include "orion/config.h"
#include "orion/log.hpp"

#include <vulkan/vk_enum_string_helper.h>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <utility>
#include <vector>

#ifndef ORION_VK_DEBUG
    #ifdef ORION_BUILD_DEBUG
        #define ORION_VK_DEBUG 1
    #else
        #define ORION_VK_DEBUG 0
    #endif
#endif

namespace orion
{
    static constexpr auto orion_vk_version = VK_MAKE_API_VERSION(0, ORION_VERSION_MAJOR, ORION_VERSION_MINOR, ORION_VERSION_PATCH);
    static constexpr auto vulkan_api_version = VK_API_VERSION_1_3;

    static constexpr auto enabled_debug_message_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
    static constexpr auto enabled_debug_message_types = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                                        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;

    static VkBool32 debug_message_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT /* messageTypes */,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* /* pUserData */)
    {
        if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            ORION_RENDERER_LOG_ERROR("[Vulkan] {}", pCallbackData->pMessage);
        } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            ORION_RENDERER_LOG_WARN("[Vulkan] {}", pCallbackData->pMessage);
        } else {
            ORION_RENDERER_LOG_TRACE("[Vulkan] {}", pCallbackData->pMessage);
        }
        return VK_FALSE;
    }

    tl::expected<VulkanInstance, VkResult> VulkanInstance::create()
    {
        // Initialize volk
        if (VkResult err = volkInitialize()) {
            ORION_RENDERER_LOG_ERROR("volkInitialize() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        }

        // Set enabled instance extensions
        std::vector<const char*> enabled_extensions;
        // GLFW required extensions
        std::uint32_t glfw_extension_count;
        const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        enabled_extensions.insert(enabled_extensions.begin(), glfw_extensions, glfw_extensions + glfw_extension_count);
        // Debug only extensions
        if constexpr (ORION_VK_DEBUG) {
            enabled_extensions.push_back("VK_EXT_debug_utils");
        }

        // Set enabled instance layers
        std::vector<const char*> enabled_layers;
        // Debug only layers
        if constexpr (ORION_VK_DEBUG) {
            enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
        }

        // Create VkInstance
        const auto app_info = VkApplicationInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Orion",
            .applicationVersion = orion_vk_version,
            .pEngineName = "OrionEngine",
            .engineVersion = orion_vk_version,
            .apiVersion = vulkan_api_version,
        };
        const auto instance_info = VkInstanceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .pApplicationInfo = &app_info,
            .enabledLayerCount = static_cast<std::uint32_t>(enabled_layers.size()),
            .ppEnabledLayerNames = enabled_layers.data(),
            .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
            .ppEnabledExtensionNames = enabled_extensions.data(),
        };
        VkInstance instance = VK_NULL_HANDLE;
        if (VkResult err = vkCreateInstance(&instance_info, nullptr, &instance)) {
            ORION_RENDERER_LOG_ERROR("vkCreateInstance() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        } else {
            ORION_RENDERER_LOG_INFO("Created VkInstance {}", fmt::ptr(instance));
            volkLoadInstance(instance);
        }

        // Create debug messenger if vulkan debugging/validation is enabled
        VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
        if constexpr (ORION_VK_DEBUG) {
            const auto debug_messenger_info = VkDebugUtilsMessengerCreateInfoEXT{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .pNext = nullptr,
                .flags = {},
                .messageSeverity = enabled_debug_message_severity,
                .messageType = enabled_debug_message_types,
                .pfnUserCallback = debug_message_callback,
            };
            if (VkResult err = vkCreateDebugUtilsMessengerEXT(instance, &debug_messenger_info, nullptr, &debug_messenger)) {
                ORION_RENDERER_LOG_ERROR("vkCreateDebugUtilsMessengerEXT() failed: {}", string_VkResult(err));
                vkDestroyInstance(instance, nullptr);
                return tl::unexpected(err);
            } else {
                ORION_RENDERER_LOG_INFO("Created VkDebugUtilsMessengerEXT {}", fmt::ptr(debug_messenger));
            }
        }

        return VulkanInstance{instance, debug_messenger};
    }

    VulkanInstance::VulkanInstance(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger)
        : vk_instance(instance)
        , vk_debug_messenger(debug_messenger)
    {
    }

    VulkanInstance::VulkanInstance(VulkanInstance&& other) noexcept
        : vk_instance(std::exchange(other.vk_instance, VK_NULL_HANDLE))
        , vk_debug_messenger(std::exchange(other.vk_debug_messenger, VK_NULL_HANDLE))
    {
    }

    VulkanInstance& VulkanInstance::operator=(VulkanInstance&& other) noexcept
    {
        if (this != &other) {
            vk_instance = std::exchange(other.vk_instance, VK_NULL_HANDLE);
            vk_debug_messenger = std::exchange(other.vk_debug_messenger, VK_NULL_HANDLE);
        }
        return *this;
    }

    VulkanInstance::~VulkanInstance()
    {
        if (vk_debug_messenger) {
            vkDestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_messenger, nullptr);
            ORION_RENDERER_LOG_INFO("Destroyed VkDebugUtilsMessengerEXT {}", fmt::ptr(vk_debug_messenger));
        }
        if (vk_instance) {
            vkDestroyInstance(vk_instance, nullptr);
            ORION_RENDERER_LOG_INFO("Destroyed VkInstance {}", fmt::ptr(vk_instance));
        }
    }

    tl::expected<std::vector<VkPhysicalDevice>, VkResult> VulkanInstance::enumerate_physical_devices()
    {
        std::uint32_t count = 0;
        if (VkResult err = vkEnumeratePhysicalDevices(vk_instance, &count, nullptr)) {
            ORION_RENDERER_LOG_ERROR("vkEnumeratePhysicalDevices() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        }
        std::vector<VkPhysicalDevice> physical_devices(count);
        if (VkResult err = vkEnumeratePhysicalDevices(vk_instance, &count, physical_devices.data())) {
            ORION_RENDERER_LOG_ERROR("vkEnumeratePhysicalDevices() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        }
        return physical_devices;
    }

    tl::expected<VulkanDevice, VkResult> VulkanInstance::create_device(VkPhysicalDevice physical_device)
    {
        auto physical_device_properties = VkPhysicalDeviceProperties2{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        vkGetPhysicalDeviceProperties2(physical_device, &physical_device_properties);
        ORION_RENDERER_LOG_INFO("Using {} to create VkDevice", physical_device_properties.properties.deviceName);

        // Get queue family properties
        std::uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties2> queue_families(queue_family_count, VkQueueFamilyProperties2{.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_family_count, queue_families.data());
        ORION_RENDERER_LOG_DEBUG("Found {} queue families...", queue_family_count);
        std::ranges::for_each(queue_families, [index = 0u](const VkQueueFamilyProperties2& queue_properties) mutable {
            ORION_RENDERER_LOG_DEBUG("Queue family {}: {{ flags: {}, queueCount: {} }}",
                                     index++,
                                     string_VkQueueFlags(queue_properties.queueFamilyProperties.queueFlags),
                                     queue_properties.queueFamilyProperties.queueCount);
        });

        // Find a queue that supports graphics && presentation
        std::uint32_t graphics_queue_family_index = UINT32_MAX;
        for (std::uint32_t i = 0; i < queue_family_count; ++i) {
            if (queue_families[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
                glfwGetPhysicalDevicePresentationSupport(vk_instance, physical_device, i)) {
                graphics_queue_family_index = i;
                break;
            }
        }
        if (graphics_queue_family_index == UINT32_MAX) {
            ORION_RENDERER_LOG_ERROR("Failed to find a queue family with VK_QUEUE_GRAPHICS_BIT && glfwGetPhysicalDevicePresentationSupport()");
            return tl::unexpected(VK_ERROR_INITIALIZATION_FAILED);
        } else {
            ORION_RENDERER_LOG_DEBUG("Using queue family {} for graphics & presentation", graphics_queue_family_index);
        }

        // Create a single graphics/presentation queue
        const auto queue_priority = 1.0f;
        const auto queue_info = VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .queueFamilyIndex = graphics_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };

        // Enabled device extensions
        std::vector<const char*> enabled_extensions;
        enabled_extensions.push_back("VK_KHR_swapchain");

        // Create device
        const auto device_info = VkDeviceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queue_info,
            .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
            .ppEnabledExtensionNames = enabled_extensions.data(),
        };
        VkDevice device = VK_NULL_HANDLE;
        if (VkResult err = vkCreateDevice(physical_device, &device_info, nullptr, &device)) {
            ORION_RENDERER_LOG_ERROR("vkCreateDevice() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        } else {
            ORION_RENDERER_LOG_INFO("Created VkDevice {}", fmt::ptr(device));
            volkLoadDevice(device);
        }

        return VulkanDevice{device, physical_device, vk_instance, graphics_queue_family_index};
    }

    VulkanDevice::VulkanDevice(
        VkDevice device,
        VkPhysicalDevice physical_device,
        VkInstance instance,
        std::uint32_t graphics_queue_family)
        : vk_device(device)
        , vk_physical_device(physical_device)
        , vk_instance(instance)
        , graphics_queue_family(graphics_queue_family)
    {
    }

    VulkanDevice::VulkanDevice(VulkanDevice&& other) noexcept
        : vk_device(std::exchange(other.vk_device, VK_NULL_HANDLE))
        , vk_physical_device(other.vk_physical_device)
        , vk_instance(other.vk_instance)
        , graphics_queue_family(other.graphics_queue_family)
    {
    }

    VulkanDevice& VulkanDevice::operator=(VulkanDevice&& other) noexcept
    {
        if (this != &other) {
            vk_device = std::exchange(other.vk_device, VK_NULL_HANDLE);
            vk_physical_device = other.vk_physical_device;
            vk_instance = other.vk_instance;
            graphics_queue_family = other.graphics_queue_family;
        }
        return *this;
    }

    VulkanDevice::~VulkanDevice()
    {
        if (vk_device != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(vk_device);
            vkDestroyDevice(vk_device, nullptr);
            ORION_RENDERER_LOG_INFO("Destroyed VkDevice {}", fmt::ptr(vk_device));
        }
    }
} // namespace orion
