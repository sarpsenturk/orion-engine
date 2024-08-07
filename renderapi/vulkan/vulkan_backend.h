#pragma once

#include "orion-renderapi/render_backend.h"

#include "vulkan_headers.h"
#include "vulkan_types.h"

#include <vector>

namespace orion::vulkan
{
    class VulkanBackend : public RenderBackend
    {
    public:
        VulkanBackend() noexcept; // Must be noexcept! It'll be called from an extern C function

        // Public API overrides
        [[nodiscard]] const char* name() const noexcept override { return "Vulkan 1.2"; }

        [[nodiscard]] auto instance() const noexcept { return instance_.get(); }

    private:
        // Private API overrides
        std::vector<PhysicalDeviceDesc> enumerate_physical_devices_api() override;
        std::unique_ptr<RenderDevice> create_device_api(std::uint32_t physical_device_index) override;

        // Internal helpers
        static constexpr auto debug_message_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        static constexpr auto debug_message_type = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        // Debug message logging callback
        static VkBool32 debug_message_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
            VkDebugUtilsMessageTypeFlagsEXT message_types,
            const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
            void* user_data);

        UniqueVkInstance create_instance() const noexcept;
        UniqueVkDebugUtilsMessengerEXT create_debug_messenger() const noexcept;
        std::vector<const char*> enabled_instance_extensions() const;
        std::vector<const char*> enabled_instance_layers() const;

        UniqueVkInstance instance_ = VK_NULL_HANDLE;
        UniqueVkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;

        std::vector<VkPhysicalDevice> physical_devices_;
        std::vector<PhysicalDeviceDesc> physical_device_descriptions_;
    };
} // namespace orion::vulkan
