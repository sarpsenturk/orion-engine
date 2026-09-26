#include "gpu.hpp"

#include "orion/core/assert.hpp"
#include "orion/core/logger.hpp"

#include <Volk/volk.h>
#include <vulkan/vk_enum_string_helper.h>

#include <GLFW/glfw3.h>

#include <cstdint>
#include <stdexcept>

#define GPU_CHECK(vkcall)                                                                          \
    do {                                                                                           \
        if (VkResult result = (vkcall); result != VK_SUCCESS) [[unlikely]] {                       \
            throw std::runtime_error(fmt::format(#vkcall " failed: {}", string_VkResult(result))); \
        }                                                                                          \
    } while (0)

namespace orion::gpu
{
    static constexpr auto vulkan_api_version = VK_API_VERSION_1_3;

    struct Device {
        VkInstance instance = VK_NULL_HANDLE;
    };


    Device* create_device(const DeviceDesc& /*desc*/)
    {
        // Initialize Volk and create VkInstance
        GPU_CHECK(volkInitialize());
        std::uint32_t glfw_extension_count;
        const auto* glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        const auto app_info         = VkApplicationInfo{
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName   = "Orion Engine",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName        = "Orion Engine",
            .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion         = vulkan_api_version,
        };
        const auto instance_info = VkInstanceCreateInfo{
            .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo        = &app_info,
            .enabledExtensionCount   = glfw_extension_count,
            .ppEnabledExtensionNames = glfw_extensions,
        };
        VkInstance instance;
        GPU_CHECK(vkCreateInstance(&instance_info, nullptr, &instance));
        volkLoadInstance(instance);

        return new Device{.instance = instance};
    }

    void destroy_device(Device* device)
    {
        ORION_ASSERT(device != nullptr);

        vkDestroyInstance(device->instance, nullptr);

        delete device;
    }

    const char* backend_name()
    {
        return "Vulkan";
    }

    const char* backend_version()
    {
        static const auto version = []() {
            const auto api_version = volkGetInstanceVersion();
            const auto major       = VK_VERSION_MAJOR(api_version);
            const auto minor       = VK_VERSION_MINOR(api_version);
            const auto patch       = VK_VERSION_PATCH(api_version);
            return fmt::format("{}.{}.{}", major, minor, patch);
        }();
        return version.c_str();
    }
} // namespace orion::gpu
