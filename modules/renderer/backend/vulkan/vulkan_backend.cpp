#include "vulkan_backend.h"

#include <algorithm>                   // std::find_if, std::all_of
#include <cstring>                     // std::strcmp
#include <orion-utils/static_vector.h> // static_vector
#include <spdlog/spdlog.h>             // SPDLOG_*
#include <utility>                     // std::exchange

extern "C" ORION_EXPORT orion::RenderBackend* create_render_backend()
{
    try {
        return new orion::vulkan::VulkanBackend();
    } catch (const orion::VulkanException& vulkan_error) {
        SPDLOG_ERROR("VulkanInitErr: {} (VkResult: {})", vulkan_error.what(), vulkan_error.result());
    } catch (const std::runtime_error& err) {
        SPDLOG_ERROR("runtime_error: {}", err.what());
    }
    return nullptr;
}

namespace orion::vulkan
{
    static constexpr auto get_required_layers() noexcept
    {
        constexpr auto max_layers = 2;
        static_vector<const char*, max_layers> layers;
        if constexpr (debug_build) {
            layers.push_back("VK_LAYER_KHRONOS_validation");
        }
        return layers;
    }

    static constexpr auto get_required_extensions() noexcept
    {
        constexpr auto max_extensions = 2;
        static_vector<const char*, max_extensions> extensions;
        if constexpr (debug_build) {
            extensions.push_back("VK_EXT_debug_utils");
        }
        if constexpr (ORION_VULKAN_SWAPCHAIN_SUPPORT) {
            extensions.push_back("VK_KHR_surface");
        }
        return extensions;
    }

    static std::vector<VkLayerProperties> get_supported_layers()
    {
        std::vector<VkLayerProperties> layers;
        std::uint32_t count = 0;
        vk_result_check(vkEnumerateInstanceLayerProperties(&count, nullptr));
        layers.resize(count);
        vk_result_check(vkEnumerateInstanceLayerProperties(&count, layers.data()));
        return layers;
    }

    static std::vector<VkExtensionProperties> get_supported_extensions()
    {
        std::vector<VkExtensionProperties> extensions;
        std::uint32_t count = 0;
        vk_result_check(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
        extensions.resize(count);
        vk_result_check(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()));
        return extensions;
    }

    VulkanBackend::VulkanBackend()
    {
        SPDLOG_TRACE("Creating Vulkan instance...");

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

        const auto enabled_layers = get_required_layers();
        // Check if all requested layers are supported
        {
            const auto supported_layers = get_supported_layers();
            SPDLOG_TRACE("Checking support for {} enabled instance layers...", enabled_layers.size());
            for (const char* layer : enabled_layers) {
                const auto pred = [layer](const VkLayerProperties& layer_properties) { return std::strcmp(layer, layer_properties.layerName) == 0; };
                const auto supported = std::ranges::find_if(supported_layers, pred) != supported_layers.end();
                if (!supported) {
                    SPDLOG_ERROR("Requested Vulkan instance layer \"{}\" is not supported!", layer);
                    throw VulkanException(VK_ERROR_LAYER_NOT_PRESENT);
                }
                SPDLOG_TRACE("-- {} ... supported", layer);
            }
            SPDLOG_TRACE("All requested instance layers supported.");
        }

        const auto enabled_extensions = get_required_extensions();
        // Check if all requested extensions are supported
        {
            const auto supported_extensions = get_supported_extensions();
            SPDLOG_TRACE("Checking support for {} enabled instance extensions...", enabled_extensions.size());
            for (const char* extension : enabled_extensions) {
                const auto pred = [extension](const VkExtensionProperties& extension_properties) { return std::strcmp(extension, extension_properties.extensionName) == 0; };
                const auto supported = std::ranges::find_if(supported_extensions, pred) != supported_extensions.end();
                if (!supported) {
                    SPDLOG_ERROR("Requested Vulkan instance extension \"{}\" is not supported", extension);
                    throw VulkanException(VK_ERROR_EXTENSION_NOT_PRESENT);
                }
                SPDLOG_TRACE("-- {} ... supported", extension);
            }
            SPDLOG_TRACE("All requested instance extensions supported.");
        }

        const VkInstanceCreateInfo instance_info{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .pApplicationInfo = &application_info,
            .enabledLayerCount = enabled_layers.size(),
            .ppEnabledLayerNames = enabled_layers.data(),
            .enabledExtensionCount = enabled_extensions.size(),
            .ppEnabledExtensionNames = enabled_extensions.data(),
        };

        vk_result_check(vkCreateInstance(&instance_info, alloc_callbacks(), &instance_));
        SPDLOG_TRACE("Created VkInstance {}", fmt::ptr(instance_));

        if constexpr (debug_build) {
            create_debug_messenger();
        }
    }

    VulkanBackend::VulkanBackend(VulkanBackend&& other) noexcept
        : instance_(std::exchange(other.instance_, VK_NULL_HANDLE))
    {
    }

    VulkanBackend& VulkanBackend::operator=(VulkanBackend&& other) noexcept
    {
        instance_ = std::exchange(other.instance_, VK_NULL_HANDLE);
        return *this;
    }

    VulkanBackend::~VulkanBackend()
    {
        // Destroy debug messenger
        if (debug_messenger_) {
            ORION_ASSERT(instance_ != VK_NULL_HANDLE); // if debug_messenger is valid instance must be valid too
            static const auto pfn_vkDestroyDebugUtilsMessengerEXT = [this]() {
                return reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT"));
            }();
            pfn_vkDestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, alloc_callbacks());
            SPDLOG_TRACE("Destroyed VkDebugUtilsMessengerEXT {}", fmt::ptr(debug_messenger_));
        }

        vkDestroyInstance(instance_, alloc_callbacks());
        SPDLOG_TRACE("Destroyed VkInstance {}", fmt::ptr(instance_));
    }

    void VulkanBackend::create_debug_messenger()
    {
        ORION_ASSERT(instance_ != VK_NULL_HANDLE); // if instance must be created before debug messenger

        // Get creation function pointer
        static const auto pfn_vkCreateDebugUtilsMessengerEXT = [this]() {
            return reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT"));
        }();

        // Create the debug messenger
        {
            const VkDebugUtilsMessengerCreateInfoEXT info{
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                .pNext = nullptr,
                .flags = 0,
                .messageSeverity = debug_message_severity,
                .messageType = debug_message_type,
                .pfnUserCallback = debug_message_callback,
                .pUserData = nullptr,
            };
            vk_result_check(pfn_vkCreateDebugUtilsMessengerEXT(instance_, &info, alloc_callbacks(), &debug_messenger_));
            SPDLOG_TRACE("Created VkDebugUtilsMessenger {}", fmt::ptr(debug_messenger_));
        }
    }

    VkBool32 VulkanBackend::debug_message_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                   VkDebugUtilsMessageTypeFlagsEXT,
                                                   const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                   void*)
    {
        if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            SPDLOG_ERROR("=== Vulkan Error ===");
            SPDLOG_ERROR("{}", callback_data->pMessage);
            SPDLOG_ERROR("Message ID: {}", callback_data->messageIdNumber);
            SPDLOG_ERROR("Message ID Name: {}", callback_data->pMessageIdName);
            SPDLOG_ERROR("=== Vulkan Error ===");
            SPDLOG_ERROR("Application will most likely crash now");
        } else if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            SPDLOG_WARN("=== Vulkan Warning ===");
            SPDLOG_WARN("{}", callback_data->pMessage);
            SPDLOG_WARN("Message ID: {}", callback_data->messageIdNumber);
            SPDLOG_WARN("Message ID Name: {}", callback_data->pMessageIdName);
            SPDLOG_WARN("=== Vulkan Warning ===");
        } else if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            SPDLOG_INFO("=== Vulkan Info ===");
            SPDLOG_INFO("{}", callback_data->pMessage);
            SPDLOG_INFO("Message ID: {}", callback_data->messageIdNumber);
            SPDLOG_INFO("Message ID Name: {}", callback_data->pMessageIdName);
            SPDLOG_INFO("=== Vulkan Info ===");
        }
        return VK_FALSE;
    }
} // namespace orion::vulkan
