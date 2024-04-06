#include "vulkan_backend.h"

#include "vulkan_conversion.h"
#include "vulkan_device.h"
#include "vulkan_platform.h"
#include "vulkan_types.h"

#include "orion-core/version.h"

#include "orion-renderer/config.h"

#include "orion-utils/static_vector.h"

#include <algorithm>
#include <cstring>
#include <span>
#include <spdlog/spdlog.h>
#include <unordered_set>
#include <utility>

extern "C" ORION_RENDER_API orion::RenderBackend* create_orion_render_backend()
{
    try {
        // Initialize the vulkan backend
        return new orion::vulkan::VulkanBackend();
    } catch (const std::exception& err) {
        SPDLOG_ERROR("std::exception: {}", err.what());
    }
    return nullptr;
}

namespace orion::vulkan
{
    namespace
    {
        constexpr auto get_required_instance_layers() noexcept
        {
            constexpr auto max_layers = 2;
            static_vector<const char*, max_layers> layers;
#ifdef ORION_BUILD_DEBUG
            layers.push_back("VK_LAYER_KHRONOS_validation");
#endif // ORION_BUILD_DEBUG
            return layers;
        }

        constexpr auto get_required_instance_extensions() noexcept
        {
            constexpr auto max_extensions = 3;
            static_vector<const char*, max_extensions> extensions;
#ifdef ORION_BUILD_DEBUG
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // ORION_BUILD_DEBUG
            if constexpr (!ORION_RENDERER_HEADLESS) {
                extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
                extensions.push_back(platform_surface_ext());
            }
            return extensions;
        }

        constexpr auto get_required_device_extensions() noexcept
        {
            constexpr auto max_extensions = 2;
            static_vector<const char*, max_extensions> extensions;
            if constexpr (!ORION_RENDERER_HEADLESS) {
                extensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
            }
            return extensions;
        }

        std::vector<VkLayerProperties> get_supported_instance_layers()
        {
            std::vector<VkLayerProperties> layers;
            std::uint32_t count = 0;
            vk_result_check(vkEnumerateInstanceLayerProperties(&count, nullptr));
            layers.resize(count);
            vk_result_check(vkEnumerateInstanceLayerProperties(&count, layers.data()));
            return layers;
        }

        std::vector<VkExtensionProperties> get_supported_instance_extensions()
        {
            std::vector<VkExtensionProperties> extensions;
            std::uint32_t count = 0;
            vk_result_check(vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr));
            extensions.resize(count);
            vk_result_check(vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data()));
            return extensions;
        }

        std::vector<VkExtensionProperties> get_supported_device_extensions(VkPhysicalDevice physical_device)
        {
            std::uint32_t count = 0;
            vk_result_check(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr));
            std::vector<VkExtensionProperties> extensions(count);
            vk_result_check(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, extensions.data()));
            return extensions;
        }

        bool check_extension_supported(std::span<const VkExtensionProperties> supported_extensions, const char* extension)
        {
            return std::ranges::find_if(supported_extensions, [extension](const auto& ext) {
                       return std::strcmp(ext.extensionName, extension) == 0;
                   }) != supported_extensions.end();
        }

        bool check_layer_support(std::span<const VkLayerProperties> supported_layers, const char* layer)
        {
            return std::ranges::find_if(supported_layers, [layer](const auto& lay) {
                       return std::strcmp(lay.layerName, layer) == 0;
                   }) != supported_layers.end();
        }

        std::vector<VkQueueFamilyProperties> get_queue_family_properties(VkPhysicalDevice physical_device)
        {
            std::uint32_t count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);
            std::vector<VkQueueFamilyProperties> queue_families(count);
            vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, queue_families.data());
            return queue_families;
        }

        std::uint32_t get_best_queue_family(std::span<const VkQueueFamilyProperties> queue_family_properties, VkQueueFlagBits requested) noexcept
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
    } // namespace

    VulkanBackend::VulkanBackend() noexcept
        : RenderBackend("orion-vulkan")
        , instance_(create_instance())
        , debug_messenger_(create_debug_messenger())
    {
    }

    std::vector<PhysicalDeviceDesc> VulkanBackend::enumerate_physical_devices_api()
    {
        ORION_ASSERT(instance() != VK_NULL_HANDLE);

        // Get the physical devices and descriptions first time
        if (physical_devices_.empty()) {
            SPDLOG_LOGGER_TRACE(logger(), "Enumerating physical devices...");
            // Enumerate the physical devices
            {
                std::uint32_t count = 0;
                vk_result_check(vkEnumeratePhysicalDevices(instance(), &count, nullptr));
                physical_devices_.resize(count);
                vk_result_check(vkEnumeratePhysicalDevices(instance(), &count, physical_devices_.data()));
            }

            // Enumerate the device properties
            physical_device_descriptions_.reserve(physical_devices_.size());
            for (physical_device_index_t index = 0; const auto& physical_device : physical_devices_) {
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

        SPDLOG_LOGGER_TRACE(logger(), "Creating VkDevice...");

        // Get the selected physical device
        auto physical_device = physical_devices_[physical_device_index];

        // Get the queue families of the physical device
        const auto queue_family_props = get_queue_family_properties(physical_device);
        ORION_ASSERT(!queue_family_props.empty());
        SPDLOG_LOGGER_TRACE(logger(), "Found {} queue families:", queue_family_props.size());
        for (std::uint32_t index = 0; const auto& queue_family : queue_family_props) {
            SPDLOG_LOGGER_TRACE(logger(), "-- Queue Family {}:", index++);
            SPDLOG_LOGGER_TRACE(logger(), "      Flags: {}", static_cast<VkQueueFlagBits>(queue_family.queueFlags));
            SPDLOG_LOGGER_TRACE(logger(), "      Queue count: {}", queue_family.queueCount);
        }

        // Find the best queue families
        const auto graphics_queue_index = get_best_queue_family(queue_family_props, VK_QUEUE_GRAPHICS_BIT);
        if (graphics_queue_index == UINT32_MAX) {
            SPDLOG_LOGGER_ERROR(logger(), "No queue family supporting Graphics");
            ORION_DEBUG_BREAK();
        }
        SPDLOG_LOGGER_TRACE(logger(), "Graphics queue: {}", graphics_queue_index);
        const auto compute_queue_index = get_best_queue_family(queue_family_props, VK_QUEUE_COMPUTE_BIT);
        if (compute_queue_index == UINT32_MAX) {
            SPDLOG_LOGGER_ERROR(logger(), "No queue family supporting Compute");
            ORION_DEBUG_BREAK();
        }
        SPDLOG_LOGGER_TRACE(logger(), "Compute queue: {}", compute_queue_index);
        const auto transfer_queue_index = get_best_queue_family(queue_family_props, VK_QUEUE_TRANSFER_BIT);
        if (transfer_queue_index == UINT32_MAX) {
            SPDLOG_LOGGER_ERROR(logger(), "No queue family supporting Transfer");
            ORION_DEBUG_BREAK();
        }
        SPDLOG_LOGGER_TRACE(logger(), "Transfer queue: {}", transfer_queue_index);

        // Put all queue family indices into set to ensure uniqueness
        std::vector queue_families{graphics_queue_index, compute_queue_index, transfer_queue_index};
        std::ranges::sort(queue_families);
        queue_families.erase(std::unique(queue_families.begin(), queue_families.end()), queue_families.end());
        SPDLOG_LOGGER_TRACE(logger(), "Using {} unique queue families", queue_families.size());

        std::vector<const char*> enabled_extensions;
        const auto supported_extensions = get_supported_device_extensions(physical_device);

        const auto required_extensions = get_required_device_extensions();
        SPDLOG_LOGGER_TRACE(logger(), "Checking support for required Vulkan device extensions...");
        for (const char* extension : required_extensions) {
            if (!check_extension_supported(supported_extensions, extension)) {
                SPDLOG_LOGGER_ERROR(logger(), "-- {} ... NOT FOUND. Aborting", extension);
                throw std::runtime_error("Failed to find Vulkan extension");
            }
            SPDLOG_LOGGER_TRACE(logger(), "-- {} ... found", extension);
            enabled_extensions.push_back(extension);
        }
        SPDLOG_LOGGER_TRACE(logger(), "Done.");

        // Must declare here to avoid dangling pointers
        std::vector<float> queue_priorities(queue_families.size(), 1.f);

        // Create the queue create info structures
        const auto queue_infos = [&queue_families, queue_priorities = queue_priorities.data()]() {
            std::vector<VkDeviceQueueCreateInfo> queue_infos;
            queue_infos.reserve(queue_families.size());
            for (auto family_index : queue_families) {
                queue_infos.push_back(VkDeviceQueueCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .queueFamilyIndex = family_index,
                    .queueCount = 1,
                    .pQueuePriorities = queue_priorities,
                });
            }
            return queue_infos;
        }();

        // Create the device
        VkDevice device = VK_NULL_HANDLE;
        {
            const auto device_info = VkDeviceCreateInfo{
                .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueCreateInfoCount = static_cast<std::uint32_t>(queue_infos.size()),
                .pQueueCreateInfos = queue_infos.data(),
                .enabledLayerCount = 0,
                .ppEnabledLayerNames = nullptr,
                .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
                .ppEnabledExtensionNames = enabled_extensions.data(),
                .pEnabledFeatures = nullptr,
            };
            vk_result_check(vkCreateDevice(physical_device, &device_info, alloc_callbacks(), &device));
        }

        // Get the created queues
        VkQueue graphics_queue = VK_NULL_HANDLE;
        vkGetDeviceQueue(device, graphics_queue_index, 0, &graphics_queue);
        VkQueue compute_queue = VK_NULL_HANDLE;
        vkGetDeviceQueue(device, compute_queue_index, 0, &compute_queue);
        VkQueue transfer_queue = VK_NULL_HANDLE;
        vkGetDeviceQueue(device, transfer_queue_index, 0, &transfer_queue);

        // Return the vulkan device
        const auto vulkan_queues = VulkanQueues{
            .graphics = {
                .family = graphics_queue_index,
                .queue = graphics_queue,
            },
            .compute = {
                .family = compute_queue_index,
                .queue = compute_queue,
            },
            .transfer = {
                .family = transfer_queue_index,
                .queue = transfer_queue,
            },
        };
        return std::make_unique<VulkanDevice>(logger(),
                                              instance(),
                                              physical_device,
                                              unique(device),
                                              vulkan_queues);
    }

    VkBool32 VulkanBackend::debug_message_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                   VkDebugUtilsMessageTypeFlagsEXT,
                                                   const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                   void* user_data)
    {
        static constexpr const char* validation_error_format = R"(
--- Vulkan Validation Message Begin ---
- Message ID: {}
- {}
-- Vulkan Validation Message End ---
)";
        const auto level = [message_severity]() {
            if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
                return spdlog::level::err;
            } else if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
                return spdlog::level::warn;
            } else if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
                return spdlog::level::info;
            }
            return spdlog::level::level_enum::off;
        }();
        auto* logger = static_cast<spdlog::logger*>(user_data);
        SPDLOG_LOGGER_CALL(logger, level, validation_error_format,
                           callback_data->pMessageIdName,
                           callback_data->pMessage);
        return VK_FALSE;
    }

    UniqueVkInstance VulkanBackend::create_instance() const noexcept
    {
        try {
            const auto vulkan_version = to_vulkan_version(engine_version);
            const auto application_info = VkApplicationInfo{
                .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                .pNext = nullptr,
                .pApplicationName = "OrionEngineApp",
                .applicationVersion = vulkan_version,
                .pEngineName = "OrionEngine",
                .engineVersion = vulkan_version,
                .apiVersion = vulkan_api_version,
            };

            const auto enabled_layers = enabled_instance_layers();
            const auto enabled_extensions = enabled_instance_extensions();

            // Create vulkan instance
            VkInstance instance = VK_NULL_HANDLE;
            {
                const auto instance_info = VkInstanceCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .pApplicationInfo = &application_info,
                    .enabledLayerCount = static_cast<std::uint32_t>(enabled_layers.size()),
                    .ppEnabledLayerNames = enabled_layers.data(),
                    .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
                    .ppEnabledExtensionNames = enabled_extensions.data(),
                };
                vk_result_check(vkCreateInstance(&instance_info, alloc_callbacks(), &instance));
            }
            return unique(instance);
        } catch (const std::exception& exception) {
            SPDLOG_LOGGER_ERROR(logger(), "exception thrown when creating vulkan backend: {}", exception.what());
            std::abort();
        } catch (...) {
            SPDLOG_LOGGER_ERROR(logger(), "exception of unknown type thrown when creating vulkan backend");
            std::abort();
        }
    }

    UniqueVkDebugUtilsMessengerEXT VulkanBackend::create_debug_messenger() const noexcept
    {
#ifndef ORION_BUILD_DEBUG
        return nullptr;
#else
        try {
            // Get creation function pointer
            auto vk_create_debug_utils_messenger_ext_fn = get_instance_proc<PFN_vkCreateDebugUtilsMessengerEXT>(instance(), "vkCreateDebugUtilsMessengerEXT");

            VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
            {
                const auto info = VkDebugUtilsMessengerCreateInfoEXT{
                    .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
                    .pNext = nullptr,
                    .flags = 0,
                    .messageSeverity = debug_message_severity,
                    .messageType = debug_message_type,
                    .pfnUserCallback = &debug_message_callback,
                    .pUserData = logger(),
                };
                vk_result_check(vk_create_debug_utils_messenger_ext_fn(instance(), &info, alloc_callbacks(), &debug_messenger));
            }
            return unique(debug_messenger, instance());
        } catch (const std::exception& exception) {
            SPDLOG_LOGGER_ERROR(logger(), "exception thrown when creating vulkan backend: {}", exception.what());
            std::abort();
        } catch (...) {
            SPDLOG_LOGGER_ERROR(logger(), "exception of unknown type thrown when creating vulkan backend");
            std::abort();
        }
#endif
    }

    std::vector<const char*> VulkanBackend::enabled_instance_extensions() const
    {
        const auto supported_extensions = get_supported_instance_extensions();

        std::vector<const char*> enabled_extensions;
        const auto required_extensions = get_required_instance_extensions();
        SPDLOG_LOGGER_TRACE(logger(), "Checking support for required Vulkan instance extensions...");
        for (const char* extension : required_extensions) {
            if (!check_extension_supported(supported_extensions, extension)) {
                SPDLOG_LOGGER_ERROR(logger(), "-- {} ... NOT FOUND. Aborting", extension);
                throw std::runtime_error("Failed to find Vulkan instance extension");
            }
            SPDLOG_LOGGER_TRACE(logger(), "-- {} ... found", extension);
            enabled_extensions.push_back(extension);
        }
        return enabled_extensions;
    }

    std::vector<const char*> VulkanBackend::enabled_instance_layers() const
    {
        const auto supported_layers = get_supported_instance_layers();

        std::vector<const char*> enabled_layers;
        const auto required_layers = get_required_instance_layers();
        SPDLOG_LOGGER_TRACE(logger(), "Checking support for required Vulkan instance layers...");
        for (const char* layer : required_layers) {
            if (!check_layer_support(supported_layers, layer)) {
                SPDLOG_LOGGER_ERROR(logger(), "-- {} ... NOT FOUND. Aborting", layer);
                throw std::runtime_error("Failed to find Vulkan layer");
            }
            SPDLOG_LOGGER_TRACE(logger(), "-- {} ... found", layer);
            enabled_layers.push_back(layer);
        }
        SPDLOG_LOGGER_TRACE(logger(), "Done.");
        return enabled_layers;
    }
} // namespace orion::vulkan
