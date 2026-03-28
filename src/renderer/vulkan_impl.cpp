#include "vulkan_impl.hpp"

#include "orion/config.h"
#include "orion/log.hpp"

#include <vulkan/vk_enum_string_helper.h>

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
} // namespace orion
