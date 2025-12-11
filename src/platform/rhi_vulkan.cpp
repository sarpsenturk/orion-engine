#include "orion/rhi/command.hpp"
#include "orion/rhi/device.hpp"
#include "orion/rhi/format.hpp"
#include "orion/rhi/rhi.hpp"
#include "orion/rhi/swapchain.hpp"

#include "orion/assert.hpp"
#include "orion/log.hpp"

#include <volk.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vk_enum_string_helper.h>

#include "platform_glfw.hpp"
#include <GLFW/glfw3.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <vector>

namespace orion
{
    namespace
    {
        constexpr auto vulkan_api_version = VK_API_VERSION_1_3;

        VkFormat to_vk_format(RHIFormat format)
        {
            switch (format) {
                case RHIFormat::B8G8R8A8_Unorm_Srgb:
                    return VK_FORMAT_B8G8R8A8_SRGB;
            }
            unreachable();
        }

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

        struct VulkanQueue {
            std::uint32_t family;
            VkQueue queue;
        };

        class RHIVulkanCommandQueue : public RHICommandQueue
        {
        public:
            RHIVulkanCommandQueue(const std::uint32_t& queue_family, const VkQueue& queue)
                : queue_family_(queue_family)
                , queue_(queue)
            {
            }

        private:
            std::uint32_t queue_family_;
            VkQueue queue_;
        };

        class RHIVulkanSwapchain : public RHISwapchain
        {
        public:
            RHIVulkanSwapchain(VkInstance instance, VkDevice device, VkSurfaceKHR surface, VkSwapchainKHR swapchain)
                : instance_(instance)
                , device_(device)
                , surface_(surface)
                , swapchain_(swapchain)
            {
                ORION_ASSERT(instance != VK_NULL_HANDLE, "VkInstance must not be VK_NULL_HANDLE");
                ORION_ASSERT(device != VK_NULL_HANDLE, "VkDevice must not be VK_NULL_HANDLE");
                ORION_ASSERT(surface != VK_NULL_HANDLE, "VkSurfaceKHR must not be VK_NULL_HANDLE");
                ORION_ASSERT(swapchain != VK_NULL_HANDLE, "VkSwapchainKHR must not be VK_NULL_HANDLE");
            }

            ~RHIVulkanSwapchain() override
            {
                vkDestroySwapchainKHR(device_, swapchain_, nullptr);
                vkDestroySurfaceKHR(instance_, surface_, nullptr);
            }

        private:
            VkInstance instance_;
            VkDevice device_;
            VkSurfaceKHR surface_;
            VkSwapchainKHR swapchain_;
        };

        class RHIVulkanDevice : public RHIDevice
        {
        public:
            RHIVulkanDevice(
                VkInstance instance,
                VkPhysicalDevice physical_device,
                VkDevice device,
                VulkanQueue graphics_queue)
                : instance_(instance)
                , physical_device_(physical_device)
                , device_(device)
                , graphics_queue_(graphics_queue)
            {
                ORION_ASSERT(instance != VK_NULL_HANDLE, "VkInstance must not be VK_NULL_HANDLE");
                ORION_ASSERT(physical_device != VK_NULL_HANDLE, "VkPhysicalDevice must not be VK_NULL_HANDLE");
                ORION_ASSERT(device != VK_NULL_HANDLE, "VkDevice must not be VK_NULL_HANDLE");
                ORION_ASSERT(graphics_queue.queue != VK_NULL_HANDLE, "VkQueue (graphics) must not be VK_NULL_HANDLE");
            }

            ~RHIVulkanDevice() override
            {
                vkDeviceWaitIdle(device_);
                vkDestroyDevice(device_, nullptr);
                ORION_CORE_LOG_INFO("Destroyed VkDevice {}", (void*)device_);
            }

        private:
            std::unique_ptr<RHICommandQueue> create_command_queue_api(const RHICommandQueueDesc& desc) override
            {
                switch (desc.type) {
                    case RHICommandQueueType::Graphics:
                        return std::make_unique<RHIVulkanCommandQueue>(graphics_queue_.family, graphics_queue_.queue);
                }
                ORION_CORE_LOG_ERROR("Invalid queue type");
                unreachable();
            }

            std::unique_ptr<RHISwapchain> create_swapchain_api(const RHISwapchainDesc& desc) override
            {
                // Create surface
                GLFWwindow* window = desc.window->window;
                VkSurfaceKHR surface = VK_NULL_HANDLE;
                if (VkResult err = glfwCreateWindowSurface(instance_, window, nullptr, &surface)) {
                    ORION_CORE_LOG_ERROR("Failed to create VkSurface with GLFWwindow* {}: {}", (void*)window, string_VkResult(err));
                    return nullptr;
                }
                ORION_CORE_LOG_INFO("Created VkSurfaceKHR {} with GLFWwindow* {}", (void*)surface, (void*)window);

                // Get surface capabilties
                VkSurfaceCapabilitiesKHR surface_capabilities;
                if (VkResult err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface, &surface_capabilities)) {
                    ORION_CORE_LOG_ERROR("Failed to get VkSurfaceKHR ({}) surface capabilities: {}", (void*)surface, string_VkResult(err));
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                    return nullptr;
                }

                // Check if requested dimensions match currentExtent
                if (desc.width != surface_capabilities.currentExtent.width || desc.height != surface_capabilities.currentExtent.height) {
                    ORION_CORE_LOG_ERROR("Requested dimensions ({}, {}) do not match currentExtent ({}, {}) of VkSurfaceKHR ({})",
                                         desc.width, desc.height,
                                         surface_capabilities.currentExtent.width, surface_capabilities.currentExtent.height,
                                         (void*)surface);
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                    return nullptr;
                }

                // Check if requested image count is supported
                const auto image_count = desc.image_count;
                if (image_count < surface_capabilities.minImageCount || image_count > surface_capabilities.maxImageCount) {
                    ORION_CORE_LOG_ERROR("Requested image count ({}) is not in range of supported image counts [{}, {}]",
                                         image_count, surface_capabilities.minImageCount, surface_capabilities.maxImageCount);
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                    return nullptr;
                }
                ORION_CORE_LOG_DEBUG("Using image_count = {} for VkSwapchainKHR", image_count);

                // Get list of supported surface formats
                std::uint32_t surface_format_count = 0;
                if (VkResult err = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface, &surface_format_count, nullptr)) {
                    ORION_CORE_LOG_ERROR("Failed to get VkPhysicalDevice {} surface formats for VkSurfaceKHR {}: {}",
                                         (void*)physical_device_, (void*)surface, string_VkResult(err));
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                    return nullptr;
                }
                std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
                if (VkResult err = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface, &surface_format_count, surface_formats.data())) {
                    ORION_CORE_LOG_ERROR("Failed to get VkPhysicalDevice {} surface formats for VkSurfaceKHR {}: {}",
                                         (void*)physical_device_, (void*)surface, string_VkResult(err));
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                    return nullptr;
                }

                // Check if requested surface format is supported
                const auto format = to_vk_format(desc.format);
                const auto colorspace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
                const auto format_cmp = [format, colorspace](const VkSurfaceFormatKHR& surface_format) {
                    return surface_format.format == format && surface_format.colorSpace == colorspace;
                };
                if (auto iter = std::ranges::find_if(surface_formats, format_cmp); iter == surface_formats.end()) {
                    ORION_CORE_LOG_ERROR("Requested surface format ({}, {}) is not supported for VkSurfaceKHR {}",
                                         string_VkFormat(format), string_VkColorSpaceKHR(colorspace), (void*)surface);
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                    return nullptr;
                }
                ORION_CORE_LOG_DEBUG("Using format = {}, {} for VkSwapchainKHR", string_VkFormat(format), string_VkColorSpaceKHR(colorspace));

                const auto swapchain_info = VkSwapchainCreateInfoKHR{
                    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
                    .pNext = nullptr,
                    .flags = {},
                    .surface = surface,
                    .minImageCount = image_count,
                    .imageFormat = format,
                    .imageColorSpace = colorspace,
                    .imageExtent = surface_capabilities.currentExtent,
                    .imageArrayLayers = 1,
                    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, // TODO: Make this customizable
                    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 0,
                    .pQueueFamilyIndices = nullptr,
                    .preTransform = surface_capabilities.currentTransform,
                    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                    .presentMode = VK_PRESENT_MODE_FIFO_KHR, // TODO: Make this customizable
                    .clipped = VK_TRUE,
                };
                VkSwapchainKHR swapchain = VK_NULL_HANDLE;
                if (VkResult err = vkCreateSwapchainKHR(device_, &swapchain_info, nullptr, &swapchain)) {
                    ORION_CORE_LOG_ERROR("Failed to create VkSwapchainKHR with VkSurface {}: {}", (void*)surface, string_VkResult(err));
                    vkDestroySurfaceKHR(instance_, surface, nullptr);
                }
                ORION_CORE_LOG_INFO("Created VkSwapchainKHR {}", (void*)swapchain);
                return std::make_unique<RHIVulkanSwapchain>(instance_, device_, surface, swapchain);
            }

            VkInstance instance_;
            VkPhysicalDevice physical_device_;
            VkDevice device_;
            VulkanQueue graphics_queue_;
        };

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
            std::unique_ptr<RHIDevice> create_device_api() override
            {
                // Enumerate physical devices
                std::uint32_t physical_device_count = 0;
                if (VkResult result = vkEnumeratePhysicalDevices(instance_, &physical_device_count, nullptr); result != VK_SUCCESS) {
                    ORION_CORE_LOG_ERROR("Failed to enumerate Vulkan physical devices: {}", string_VkResult(result));
                    return nullptr;
                }
                if (physical_device_count == 0) {
                    ORION_CORE_LOG_ERROR("No valid Vulkan physical devices found");
                    return nullptr;
                }
                std::vector<VkPhysicalDevice> physical_devices(physical_device_count);
                if (VkResult result = vkEnumeratePhysicalDevices(instance_, &physical_device_count, physical_devices.data()); result != VK_SUCCESS) {
                    ORION_CORE_LOG_ERROR("Failed to enumerate Vulkan physical devices: {}", string_VkResult(result));
                    return nullptr;
                }

                // Get physical device properties
                std::vector<VkPhysicalDeviceProperties> physical_device_properties(physical_device_count);
                std::ranges::transform(physical_devices, physical_device_properties.begin(), [](VkPhysicalDevice physical_device) {
                    VkPhysicalDeviceProperties properties;
                    vkGetPhysicalDeviceProperties(physical_device, &properties);
                    ORION_CORE_LOG_DEBUG("Found VkPhysicalDevice ({}): {} - {}", (void*)physical_device, properties.deviceName, string_VkPhysicalDeviceType(properties.deviceType));
                    return properties;
                });

                // Select physical device
                // TODO: Allow proper control over physical device selection
                VkPhysicalDevice physical_device = physical_devices[0];
                ORION_CORE_LOG_INFO("Selected VkPhysicalDevice ({}): {}", (void*)physical_device, physical_device_properties[0].deviceName);

                // Enumerate queue families
                std::uint32_t queue_family_count = 0;
                vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
                if (queue_family_count == 0) {
                    ORION_CORE_LOG_ERROR("No queue families found!");
                    return nullptr;
                }
                std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
                vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
                for (std::uint32_t idx = 0; const auto& queue_family : queue_families) {
                    ORION_CORE_LOG_DEBUG("Queue family {}: {{ flags: {}, queue_count: {} }}", idx++, string_VkQueueFlags(queue_family.queueFlags), queue_family.queueCount);
                }

                // Find graphics queue
                std::uint32_t graphics_queue_family = UINT32_MAX;
                for (std::uint32_t idx = 0; idx < queue_family_count; ++idx) {
                    if (queue_families[idx].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                        graphics_queue_family = idx;
                        break;
                    }
                }
                if (graphics_queue_family == UINT32_MAX) {
                    ORION_CORE_LOG_ERROR("Failed to find a Vulkan queue family with VK_QUEUE_GRAPHICS_BIT");
                    return nullptr;
                }

                // Queue create info
                const auto queue_priorities = std::array{1.0f};
                const auto queue_infos = std::array{
                    VkDeviceQueueCreateInfo{
                        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                        .pNext = nullptr,
                        .flags = {},
                        .queueFamilyIndex = graphics_queue_family,
                        .queueCount = 1,
                        .pQueuePriorities = queue_priorities.data(),
                    },
                };

                // Enabled device extensions
                std::vector<const char*> enabled_extensions;
                enabled_extensions.push_back("VK_KHR_swapchain");

                // Create the device
                const auto device_info = VkDeviceCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .queueCreateInfoCount = static_cast<std::uint32_t>(queue_infos.size()),
                    .pQueueCreateInfos = queue_infos.data(),
                    .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
                    .ppEnabledExtensionNames = enabled_extensions.data(),
                    .pEnabledFeatures = nullptr,
                };
                VkDevice device = VK_NULL_HANDLE;
                if (VkResult result = vkCreateDevice(physical_device, &device_info, nullptr, &device); result != VK_SUCCESS) {
                    ORION_CORE_LOG_ERROR("Failed to create a Vulkan device: {}", string_VkResult(result));
                    return nullptr;
                }
                ORION_CORE_LOG_DEBUG("Created VkDevice {}", (void*)device);

                // Retrieve created queues
                VulkanQueue graphics_queue = {.family = graphics_queue_family};
                vkGetDeviceQueue(device, graphics_queue_family, 0, &graphics_queue.queue);
                ORION_CORE_LOG_DEBUG("Got VkQueue (graphics) {}", (void*)graphics_queue.queue);

                return std::make_unique<RHIVulkanDevice>(instance_, physical_device, device, graphics_queue);
            }

            VkInstance instance_;
            VkDebugUtilsMessengerEXT debug_messenger_;
        };
    } // namespace

    std::unique_ptr<RHIInstance> rhi_vulkan_create_instance()
    {
        if (volkInitialize() != VK_SUCCESS) {
            ORION_CORE_LOG_ERROR("Failed to initialize volk. Vulkan may not be installed on your system");
            return nullptr;
        }
        glfwInitVulkanLoader(vkGetInstanceProcAddr);

        // Enabled instance layers
        std::vector<const char*> enabled_layers;
        enabled_layers.push_back("VK_LAYER_KHRONOS_validation");

        // Enabled instance extensions
        std::vector<const char*> enabled_extensions;
        enabled_extensions.push_back("VK_EXT_debug_utils");

        // Add GLFW required extensions
        std::uint32_t glfw_extension_count;
        const char** extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        for (std::uint32_t i = 0; i < glfw_extension_count; ++i) {
            enabled_extensions.push_back(extensions[i]);
        }

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

        return std::make_unique<RHIVulkanInstance>(instance, debug_messenger);
    }
} // namespace orion
