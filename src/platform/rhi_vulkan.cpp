#include "orion/rhi/rhi_instance.hpp"

#include "orion/assert.hpp"
#include "orion/log.hpp"

#include <volk.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vk_enum_string_helper.h>

#include <cstdint>
#include <vector>

namespace orion
{
    namespace
    {
        constexpr auto vulkan_api_version = VK_API_VERSION_1_3;

        VkBool32 vulkan_debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT severity,
            VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
            const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
            void* /*pUserData*/)
        {
            if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
                ORION_CORE_LOG_ERROR("Vulkan validation error ({}): {}", callback_data->pMessageIdName, callback_data->pMessage);
            } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                ORION_CORE_LOG_ERROR("Vulkan validation warning ({}): {}", callback_data->pMessageIdName, callback_data->pMessage);
            } else {
                ORION_CORE_LOG_TRACE("Vulkan validation message ({}): {}", callback_data->pMessageIdName, callback_data->pMessage);
            }
            return VK_FALSE;
        }

        class RHIVulkanInstance : public RHIInstance
        {
        public:
            RHIVulkanInstance(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger)
                : instance_(instance)
                , debug_messenger_(debug_messenger)
            {
                ORION_ASSERT(instance != VK_NULL_HANDLE, "VkInstance must not be VK_NULL_HANDLE");
                ORION_ASSERT(debug_messenger != VK_NULL_HANDLE, "VkDebugUtilsMessengerEXT must not be VK_NULL_HANDLE");
            }

            ~RHIVulkanInstance() override
            {
                vkDestroyDebugUtilsMessengerEXT(instance_, debug_messenger_, nullptr);
                ORION_CORE_LOG_INFO("Destroyed VkDebugUtilsMessengerEXT {}", (void*)debug_messenger_);
                vkDestroyInstance(instance_, nullptr);
                ORION_CORE_LOG_INFO("Destroyed VkInstance {}", (void*)instance_);
            }

        private:
            VkInstance instance_;
            VkDebugUtilsMessengerEXT debug_messenger_;
        };
    } // namespace

    RHIInstance* rhi_vulkan_create_instance()
    {
        if (volkInitialize() != VK_SUCCESS) {
            ORION_CORE_LOG_ERROR("Failed to initialize volk. Vulkan may not be installed on your system");
            return nullptr;
        }

        std::vector<const char*> enabled_layers;
        enabled_layers.push_back("VK_LAYER_KHRONOS_validation");
        std::vector<const char*> enabled_extensions;
        enabled_extensions.push_back("VK_EXT_debug_utils");

        const auto app_info = VkApplicationInfo{
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pNext = nullptr,
            .pApplicationName = "Orion",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "Orion",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
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
        if (VkResult result = vkCreateInstance(&instance_info, nullptr, &instance); result != VK_SUCCESS) {
            ORION_CORE_LOG_ERROR("Failed to create Vulkan instance: {}", string_VkResult(result));
            return nullptr;
        }
        volkLoadInstance(instance);
        ORION_CORE_LOG_INFO("Created VkInstance {}", (void*)instance);

        const auto debug_messenger_info = VkDebugUtilsMessengerCreateInfoEXT{
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .pNext = nullptr,
            .flags = {},
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
            .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = vulkan_debug_callback,
        };
        VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
        if (VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &debug_messenger_info, nullptr, &debug_messenger); result != VK_SUCCESS) {
            ORION_CORE_LOG_ERROR("Failed to created Vulkan debug messenger: {}", string_VkResult(result));
            return nullptr;
        }
        ORION_CORE_LOG_INFO("Created VkDebugUtilsMessengerEXT {}", (void*)debug_messenger);

        return new RHIVulkanInstance(instance, debug_messenger);
    }

    void rhi_vulkan_destroy_instance(RHIInstance* instance)
    {
        delete instance;
    }
} // namespace orion
