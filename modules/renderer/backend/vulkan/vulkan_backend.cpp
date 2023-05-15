#include "vulkan_backend.h"

#include "vulkan_conversion.h"
#include "vulkan_device.h"
#include "vulkan_platform.h"
#include "vulkan_types.h"

#include <algorithm>                   // std::ranges::find_if
#include <cstring>                     // std::strcmp
#include <orion-utils/static_vector.h> // static_vector
#include <span>                        // std::span
#include <spdlog/spdlog.h>             // SPDLOG_LOGGER_*
#include <unordered_set>               // std::unordered_set
#include <utility>                     // std::exchange

extern "C" ORION_RENDER_API orion::RenderBackend* create_render_backend()
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
        constexpr auto get_required_layers() noexcept
        {
            constexpr auto max_layers = 2;
            static_vector<const char*, max_layers> layers;
            if constexpr (debug_build) {
                layers.push_back("VK_LAYER_KHRONOS_validation");
            }
            return layers;
        }

        constexpr auto get_required_extensions() noexcept
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

        constexpr auto get_required_device_extensions() noexcept
        {
            constexpr auto max_extensions = 2;
            static_vector<const char*, max_extensions> extensions;
            if constexpr (ORION_VULKAN_SWAPCHAIN_SUPPORT) {
                extensions.push_back("VK_KHR_swapchain");
            }
            return extensions;
        }
    } // namespace

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
        : RenderBackend("orion-vulkan")
        , instance_(create_vk_instance(get_required_layers(), get_required_extensions()))
    {
        // Create vulkan debug utils if debug mode is enabled
        if constexpr (debug_build) {
            debug_messenger_ = create_vk_debug_utils_messenger(instance_.get(), debug_message_severity, debug_message_type, debug_message_callback, logger());
        }
    }

    std::vector<PhysicalDeviceDesc> VulkanBackend::enumerate_physical_devices_api()
    {
        ORION_ASSERT(instance_ != VK_NULL_HANDLE);

        // Get the physical devices and descriptions first time
        if (physical_devices_.empty()) {
            SPDLOG_LOGGER_TRACE(logger(), "Enumerating physical devices...");
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

        SPDLOG_LOGGER_TRACE(logger(), "Creating VkDevice...");

        // Get the selected physical device
        auto physical_device = physical_devices_[physical_device_index];

        // Get the queue families of the physical device
        const auto queue_families = get_queue_family_properties(physical_device);
        ORION_ASSERT(!queue_families.empty());
        SPDLOG_LOGGER_TRACE(logger(), "Found {} queue families:", queue_families.size());
        for (std::uint32_t index = 0; const auto& queue_family : queue_families) {
            SPDLOG_LOGGER_TRACE(logger(), "-- Queue Family {}:", index);
            SPDLOG_LOGGER_TRACE(logger(), "      Flags: {}", to_string(queue_family.queueFlags));
            SPDLOG_LOGGER_TRACE(logger(), "      Queue count: {}", queue_family.queueCount);
        }

        // Find the best queue families
        const auto graphics_queue_index = get_best_queue_family(queue_families, VK_QUEUE_GRAPHICS_BIT);
        if (graphics_queue_index == UINT32_MAX) {
            SPDLOG_LOGGER_ERROR(logger(), "No queue family supporting Graphics");
            ORION_DEBUG_BREAK();
        }
        SPDLOG_LOGGER_TRACE(logger(), "Graphics queue: {}", graphics_queue_index);
        const auto compute_queue_index = get_best_queue_family(queue_families, VK_QUEUE_COMPUTE_BIT);
        if (compute_queue_index == UINT32_MAX) {
            SPDLOG_LOGGER_ERROR(logger(), "No queue family supporting Compute");
            ORION_DEBUG_BREAK();
        }
        SPDLOG_LOGGER_TRACE(logger(), "Compute queue: {}", compute_queue_index);
        const auto transfer_queue_index = get_best_queue_family(queue_families, VK_QUEUE_TRANSFER_BIT);
        if (transfer_queue_index == UINT32_MAX) {
            SPDLOG_LOGGER_ERROR(logger(), "No queue family supporting Transfer");
            ORION_DEBUG_BREAK();
        }
        SPDLOG_LOGGER_TRACE(logger(), "Transfer queue: {}", transfer_queue_index);

        // Put all queue family indices into set to ensure uniqueness
        std::vector used_queue_families{graphics_queue_index, compute_queue_index, transfer_queue_index};
        std::ranges::sort(used_queue_families);
        used_queue_families.erase(std::unique(used_queue_families.begin(), used_queue_families.end()), used_queue_families.end());
        SPDLOG_LOGGER_TRACE(logger(), "Using {} unique queue families", used_queue_families.size());

        // Create the device
        auto device = create_vk_device(physical_device, used_queue_families, get_required_device_extensions());

        // Get the created queues
        VkQueue graphics_queue = VK_NULL_HANDLE;
        vkGetDeviceQueue(device.get(), graphics_queue_index, 0, &graphics_queue);
        VkQueue compute_queue = VK_NULL_HANDLE;
        vkGetDeviceQueue(device.get(), compute_queue_index, 0, &compute_queue);
        VkQueue transfer_queue = VK_NULL_HANDLE;
        vkGetDeviceQueue(device.get(), transfer_queue_index, 0, &transfer_queue);

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
        return std::make_unique<VulkanDevice>(logger(), instance_.get(), physical_device, std::move(device), vulkan_queues);
    }

    VkBool32 VulkanBackend::debug_message_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                   VkDebugUtilsMessageTypeFlagsEXT,
                                                   const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
                                                   void* user_data)
    {
        static constexpr const char* validation_error_format = R"(
--- Vulkan Validation Error Begin ---
Message ID: {}, Message ID Name: {}
{}
-- Vulkan Validation Error End ---
)";
        auto* logger = static_cast<spdlog::logger*>(user_data);
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
        SPDLOG_LOGGER_CALL(logger, level, validation_error_format, callback_data->messageIdNumber, callback_data->pMessageIdName, callback_data->pMessage);
        return VK_FALSE;
    }
} // namespace orion::vulkan
