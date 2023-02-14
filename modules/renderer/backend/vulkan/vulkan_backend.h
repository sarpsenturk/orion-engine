#pragma once

#include "orion-core/config.h"
#include "orion-renderer/render_backend.h"
#include "vulkan_headers.h"

namespace orion::vulkan
{
    class VulkanBackend : public RenderBackend
    {
    public:
        VulkanBackend();
        VulkanBackend(const VulkanBackend&) = delete;
        VulkanBackend(VulkanBackend&& other) noexcept;
        VulkanBackend& operator=(const VulkanBackend&) = delete;
        VulkanBackend& operator=(VulkanBackend&& other) noexcept;
        ~VulkanBackend() override;

        [[nodiscard]] const char* name() const noexcept override { return "Vulkan 1.0"; }

        static constexpr auto vulkan_api_version = VK_API_VERSION_1_0;

    private:
        void create_debug_messenger();

        static constexpr auto debug_message_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
        static constexpr auto debug_message_type = VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                                   VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;

        static VkBool32 debug_message_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
            VkDebugUtilsMessageTypeFlagsEXT message_types,
            const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
            void* user_data);

        VkInstance instance_ = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;
    };
} // namespace orion::vulkan
