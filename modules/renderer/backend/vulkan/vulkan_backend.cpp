#include "vulkan_backend.h"

#include "vulkan_conversion.h"
#include "vulkan_device.h"
#include "vulkan_platform.h"
#include "vulkan_types.h"

#include <algorithm>                         // std::find_if
#include <cstring>                           // std::strcmp
#include <orion-utils/static_vector.h>       // static_vector
#include <span>                              // std::span
#include <spdlog/sinks/stdout_color_sinks.h> // spdlog::stdout_color_*
#include <spdlog/spdlog.h>                   // SPDLOG_LOGGER_*
#include <unordered_set>                     // std::unordered_set
#include <utility>                           // std::exchange

extern "C" ORION_RENDER_API orion::RenderBackend* create_render_backend()
{
    try {
        // Initialize the vulkan backend
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
    // Initialize the vulkan logger
    static const auto vulkan_logger = []() {
        auto logger = spdlog::stdout_color_mt("orion-vulkan");
        return logger;
    }();

    std::shared_ptr<spdlog::logger> logger() { return vulkan_logger; }

    spdlog::logger* logger_raw() { return vulkan_logger.get(); }

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
        constexpr auto max_extensions = 3;
        static_vector<const char*, max_extensions> extensions;
        if constexpr (debug_build) {
            extensions.push_back("VK_EXT_debug_utils");
        }
        if constexpr (ORION_VULKAN_SWAPCHAIN_SUPPORT) {
            extensions.push_back("VK_KHR_surface");
            extensions.push_back(platform_surface_ext());
        }
        return extensions;
    }

    static constexpr auto get_required_device_extensions() noexcept
    {
        constexpr auto max_extensions = 2;
        static_vector<const char*, max_extensions> extensions;
        if constexpr (ORION_VULKAN_SWAPCHAIN_SUPPORT) {
            extensions.push_back("VK_KHR_swapchain");
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

    static std::vector<VkExtensionProperties> get_supported_device_extensions(VkPhysicalDevice physical_device)
    {
        std::uint32_t count = 0;
        vk_result_check(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr));
        std::vector<VkExtensionProperties> extensions(count);
        vk_result_check(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, extensions.data()));
        return extensions;
    }

    static bool check_extensions_supported(std::span<const char* const> enabled_extensions, std::span<const VkExtensionProperties> supported_extensions)
    {
        SPDLOG_LOGGER_TRACE(logger_raw(), "Checking support for {} enabled extensions...", enabled_extensions.size());
        bool all_supported = true;
        for (const char* extension : enabled_extensions) {
            const auto pred = [extension](const VkExtensionProperties& extension_properties) { return std::strcmp(extension, extension_properties.extensionName) == 0; };
            const auto supported = std::ranges::find_if(supported_extensions, pred) != supported_extensions.end();
            if (!supported) {
                SPDLOG_LOGGER_ERROR(logger_raw(), "Requested Vulkan extension \"{}\" is not supported", extension);
                all_supported = false;
            }
            SPDLOG_LOGGER_TRACE(logger_raw(), "-- {} ... supported", extension);
        }
        SPDLOG_LOGGER_TRACE(logger_raw(), "All requested extensions supported.");
        return all_supported;
    }

    static std::vector<VkQueueFamilyProperties> get_queue_family_properties(VkPhysicalDevice physical_device)
    {
        std::uint32_t count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
        std::vector<VkQueueFamilyProperties> queue_families(count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, queue_families.data());
        return queue_families;
    }

    static std::uint32_t get_best_queue_family(std::span<const VkQueueFamilyProperties> queue_family_properties, VkQueueFlagBits requested) noexcept
    {
        std::uint32_t best_index = UINT32_MAX;
        VkQueueFlags best_score = UINT32_MAX;
        for (std::uint32_t index = 0; const auto& queue_family : queue_family_properties) {
            if (queue_family.queueFlags & requested) {
                const auto score = queue_family.queueFlags ^ requested;
                if (score < best_score) {
                    best_index = index;
                    best_score = score;
                }
            }
            ++index;
        }
        return best_index;
    }

    VulkanBackend::VulkanBackend()
    {
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Creating Vulkan instance...");

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
            SPDLOG_LOGGER_TRACE(logger_raw(), "Checking support for {} enabled instance layers...", enabled_layers.size());
            for (const char* layer : enabled_layers) {
                const auto pred = [layer](const VkLayerProperties& layer_properties) { return std::strcmp(layer, layer_properties.layerName) == 0; };
                const auto supported = std::ranges::find_if(supported_layers, pred) != supported_layers.end();
                if (!supported) {
                    SPDLOG_LOGGER_ERROR(logger_raw(), "Requested Vulkan instance layer \"{}\" is not supported!", layer);
                    throw VulkanException(VK_ERROR_LAYER_NOT_PRESENT);
                }
                SPDLOG_LOGGER_TRACE(logger_raw(), "-- {} ... supported", layer);
            }
            SPDLOG_LOGGER_TRACE(logger_raw(), "All requested instance layers supported.");
        }

        const auto enabled_extensions = get_required_extensions();
        // Check if all requested extensions are supported
        {
            const auto supported_extensions = get_supported_extensions();
            if (!check_extensions_supported(enabled_extensions, supported_extensions)) {
                throw VulkanException(VK_ERROR_EXTENSION_NOT_PRESENT);
            }
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

        VkInstance instance = VK_NULL_HANDLE;
        vk_result_check(vkCreateInstance(&instance_info, alloc_callbacks(), &instance));
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Created VkInstance {}", fmt::ptr(instance));
        instance_ = {instance, {}};

        if constexpr (debug_build) {
            create_debug_messenger();
        }
    }

    std::vector<PhysicalDeviceDesc> VulkanBackend::enumerate_physical_devices_api()
    {
        ORION_ASSERT(instance_ != VK_NULL_HANDLE);

        // Get the physical devices and descriptions first time
        if (physical_devices_.empty()) {
            SPDLOG_LOGGER_TRACE(logger_raw(), "Enumerating physical devices...");
            // Enumerate the physical devices
            {
                std::uint32_t count = 0;
                vk_result_check(vkEnumeratePhysicalDevices(instance_.get(), &count, nullptr));
                physical_devices_.resize(count);
                vk_result_check(vkEnumeratePhysicalDevices(instance_.get(), &count, physical_devices_.data()));
            }

            // Enumerate the device properties
            physical_device_descriptions_.reserve(physical_devices_.size());
            for (std::uint32_t index = 0; auto physical_device : physical_devices_) {
                VkPhysicalDeviceProperties properties;
                vkGetPhysicalDeviceProperties(physical_device, &properties);
                physical_device_descriptions_.push_back({
                    .index = index++,
                    .type = to_orion_type(properties.deviceType),
                    .name = std::string{properties.deviceName},
                });
            }
        }

        // Generate the descriptions
        return physical_device_descriptions_;
    }

    std::unique_ptr<RenderDevice> VulkanBackend::create_device_api(std::uint32_t physical_device_index)
    {
        ORION_ASSERT(physical_device_index < physical_devices_.size());

        SPDLOG_LOGGER_DEBUG(logger_raw(), "Creating VkDevice...");

        // Get the selected physical device
        auto physical_device = physical_devices_[physical_device_index];

        // Get the queue families of the physical device
        const auto queue_families = get_queue_family_properties(physical_device);
        ORION_ASSERT(!queue_families.empty());
        SPDLOG_LOGGER_TRACE(logger_raw(), "Found {} queue families:", queue_families.size());
        for (std::uint32_t index = 0; const auto& queue_family : queue_families) {
            SPDLOG_LOGGER_TRACE(logger_raw(), "-- Queue Family {}:", index);
            SPDLOG_LOGGER_TRACE(logger_raw(), "      Flags: {}", to_string(queue_family.queueFlags));
            SPDLOG_LOGGER_TRACE(logger_raw(), "      Queue count: {}", queue_family.queueCount);
        }

        // Find the best queue families
        const auto graphics_queue_index = get_best_queue_family(queue_families, VK_QUEUE_GRAPHICS_BIT);
        if (graphics_queue_index == UINT32_MAX) {
            SPDLOG_LOGGER_ERROR(logger_raw(), "No queue family supporting Graphics");
            throw VulkanException(VK_ERROR_UNKNOWN);
        }
        SPDLOG_LOGGER_TRACE(logger_raw(), "Graphics queue: {}", graphics_queue_index);
        const auto compute_queue_index = get_best_queue_family(queue_families, VK_QUEUE_COMPUTE_BIT);
        if (compute_queue_index == UINT32_MAX) {
            SPDLOG_LOGGER_ERROR(logger_raw(), "No queue family supporting Compute");
            throw VulkanException(VK_ERROR_UNKNOWN);
        }
        SPDLOG_LOGGER_TRACE(logger_raw(), "Compute queue: {}", compute_queue_index);
        const auto transfer_queue_index = get_best_queue_family(queue_families, VK_QUEUE_TRANSFER_BIT);
        if (transfer_queue_index == UINT32_MAX) {
            SPDLOG_LOGGER_ERROR(logger_raw(), "No queue family supporting Transfer");
            throw VulkanException(VK_ERROR_UNKNOWN);
        }
        SPDLOG_LOGGER_TRACE(logger_raw(), "Transfer queue: {}", transfer_queue_index);

        // Put all queue family indices into set to ensure uniqueness
        const std::unordered_set used_queue_families{graphics_queue_index, compute_queue_index, transfer_queue_index};
        SPDLOG_LOGGER_TRACE(logger_raw(), "Using {} unique queue families", used_queue_families.size());

        // Must declare here to avoid dangling pointers
        std::vector<float> queue_priorities{1.f};

        // Create the queue create info structures
        const auto queue_infos = [&used_queue_families, &queue_priorities]() {
            std::vector<VkDeviceQueueCreateInfo> queue_infos;
            queue_infos.reserve(used_queue_families.size());
            for (auto family_index : used_queue_families) {
                queue_infos.push_back({
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .queueFamilyIndex = family_index,
                    .queueCount = 1,
                    .pQueuePriorities = queue_priorities.data(),
                });
            }
            return queue_infos;
        }();

        // Enable required extensions and check for support
        const auto enabled_extensions = get_required_device_extensions();
        {
            const auto supported_extensions = get_supported_device_extensions(physical_device);
            if (!check_extensions_supported(enabled_extensions, supported_extensions)) {
                throw VulkanException(VK_ERROR_EXTENSION_NOT_PRESENT);
            }
        }

        // Create the device
        const VkDeviceCreateInfo device_info{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueCreateInfoCount = static_cast<std::uint32_t>(queue_infos.size()),
            .pQueueCreateInfos = queue_infos.data(),
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = enabled_extensions.size(),
            .ppEnabledExtensionNames = enabled_extensions.data(),
            .pEnabledFeatures = nullptr,
        };
        VkDevice device = VK_NULL_HANDLE;
        vk_result_check(vkCreateDevice(physical_device, &device_info, alloc_callbacks(), &device));
        SPDLOG_LOGGER_DEBUG(logger_raw(), "Created VkDevice {}", fmt::ptr(device));

        // Get the created queues
        VkQueue graphics_queue;
        vkGetDeviceQueue(device, graphics_queue_index, 0, &graphics_queue);
        VkQueue compute_queue;
        vkGetDeviceQueue(device, compute_queue_index, 0, &compute_queue);
        VkQueue transfer_queue;
        vkGetDeviceQueue(device, transfer_queue_index, 0, &transfer_queue);

        // Return the vulkan device
        const auto vulkan_queues = VulkanQueues{
            .graphics = {
                .index = graphics_queue_index,
                .queue = graphics_queue,
            },
            .compute = {
                .index = compute_queue_index,
                .queue = compute_queue,
            },
            .transfer = {
                .index = transfer_queue_index,
                .queue = transfer_queue,
            },
        };
        return std::make_unique<VulkanDevice>(instance_.get(), physical_device, UniqueVkDevice(device), vulkan_queues);
    }

    void VulkanBackend::create_debug_messenger()
    {
        ORION_ASSERT(instance_ != VK_NULL_HANDLE); // if instance must be created before debug messenger

        // Get creation function pointer
        static const auto pfn_vkCreateDebugUtilsMessengerEXT = [this]() {
            return reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance_.get(), "vkCreateDebugUtilsMessengerEXT"));
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

            VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
            vk_result_check(pfn_vkCreateDebugUtilsMessengerEXT(instance_.get(), &info, alloc_callbacks(), &debug_messenger));
            SPDLOG_LOGGER_TRACE(logger_raw(), "Created VkDebugUtilsMessenger {}", fmt::ptr(debug_messenger));

            debug_messenger_ = UniqueVkDebugUtilsMessengerEXT(debug_messenger, DebugUtilsMessengerDeleter{instance_.get()});
        }
    }

    VkBool32 VulkanBackend::debug_message_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                   VkDebugUtilsMessageTypeFlagsEXT,
                                                   const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                   void*)
    {
        if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            SPDLOG_LOGGER_ERROR(logger_raw(), "Message ID: {:#x}, Message ID Name: {}", callback_data->messageIdNumber, callback_data->pMessageIdName);
            SPDLOG_LOGGER_ERROR(logger_raw(), "{}", callback_data->pMessage);
            SPDLOG_LOGGER_ERROR(logger_raw(), "Application will most likely crash now");
        } else if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            SPDLOG_LOGGER_WARN(logger_raw(), "Message ID: {:#x}, Message ID Name: {}", callback_data->messageIdNumber, callback_data->pMessageIdName);
            SPDLOG_LOGGER_WARN(logger_raw(), "{}", callback_data->pMessage);
        } else if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
            SPDLOG_LOGGER_INFO(logger_raw(), "Message ID: {:#x}, Message ID Name: {}", callback_data->messageIdNumber, callback_data->pMessageIdName);
            SPDLOG_LOGGER_INFO(logger_raw(), "{}", callback_data->pMessage);
        }
        return VK_FALSE;
    }
} // namespace orion::vulkan
