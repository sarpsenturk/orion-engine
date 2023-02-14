#include "vulkan_backend.h"

#include <spdlog/spdlog.h> // SPDLOG_*
#include <utility>         // std::exchange

extern "C" ORION_EXPORT orion::RenderBackend* create_render_backend()
{
    return new orion::vulkan::VulkanBackend();
}

namespace orion::vulkan
{
    VulkanBackend::VulkanBackend()
    {
        {
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

            const VkInstanceCreateInfo instance_info{
                .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
                .pNext = nullptr,
                .flags = {},
                .pApplicationInfo = &application_info,
                .enabledLayerCount = 0,
                .ppEnabledLayerNames = nullptr,
                .enabledExtensionCount = 0,
                .ppEnabledExtensionNames = nullptr,
            };

            vk_result_check(vkCreateInstance(&instance_info, nullptr, &instance_));
            SPDLOG_TRACE("Created VkInstance {}", fmt::ptr(instance_));
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
        vkDestroyInstance(instance_, nullptr);
        SPDLOG_TRACE("Destroyed VkInstance {}", fmt::ptr(instance_));
    }
} // namespace orion::vulkan
