#include "vulkan_impl.hpp"

#include "orion/config.h"
#include "orion/log.hpp"

#include <vulkan/vk_enum_string_helper.h>

#include "../platform/platform_glfw.hpp"
#include <GLFW/glfw3.h>

#include <algorithm>
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
        // GLFW required extensions
        std::uint32_t glfw_extension_count;
        const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
        enabled_extensions.insert(enabled_extensions.begin(), glfw_extensions, glfw_extensions + glfw_extension_count);
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

    tl::expected<std::vector<VkPhysicalDevice>, VkResult> VulkanInstance::enumerate_physical_devices()
    {
        std::uint32_t count = 0;
        if (VkResult err = vkEnumeratePhysicalDevices(vk_instance, &count, nullptr)) {
            ORION_RENDERER_LOG_ERROR("vkEnumeratePhysicalDevices() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        }
        std::vector<VkPhysicalDevice> physical_devices(count);
        if (VkResult err = vkEnumeratePhysicalDevices(vk_instance, &count, physical_devices.data())) {
            ORION_RENDERER_LOG_ERROR("vkEnumeratePhysicalDevices() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        }
        return physical_devices;
    }

    tl::expected<VulkanDevice, VkResult> VulkanInstance::create_device(VkPhysicalDevice physical_device)
    {
        auto physical_device_properties = VkPhysicalDeviceProperties2{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        vkGetPhysicalDeviceProperties2(physical_device, &physical_device_properties);
        ORION_RENDERER_LOG_INFO("Using {} to create VkDevice", physical_device_properties.properties.deviceName);

        // Get queue family properties
        std::uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_family_count, nullptr);
        std::vector<VkQueueFamilyProperties2> queue_families(queue_family_count, VkQueueFamilyProperties2{.sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});
        vkGetPhysicalDeviceQueueFamilyProperties2(physical_device, &queue_family_count, queue_families.data());
        ORION_RENDERER_LOG_DEBUG("Found {} queue families...", queue_family_count);
        std::ranges::for_each(queue_families, [index = 0u](const VkQueueFamilyProperties2& queue_properties) mutable {
            ORION_RENDERER_LOG_DEBUG("Queue family {}: {{ flags: {}, queueCount: {} }}",
                                     index++,
                                     string_VkQueueFlags(queue_properties.queueFamilyProperties.queueFlags),
                                     queue_properties.queueFamilyProperties.queueCount);
        });

        // Find a queue that supports graphics && presentation
        std::uint32_t graphics_queue_family_index = UINT32_MAX;
        for (std::uint32_t i = 0; i < queue_family_count; ++i) {
            if (queue_families[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT &&
                glfwGetPhysicalDevicePresentationSupport(vk_instance, physical_device, i)) {
                graphics_queue_family_index = i;
                break;
            }
        }
        if (graphics_queue_family_index == UINT32_MAX) {
            ORION_RENDERER_LOG_ERROR("Failed to find a queue family with VK_QUEUE_GRAPHICS_BIT && glfwGetPhysicalDevicePresentationSupport()");
            return tl::unexpected(VK_ERROR_INITIALIZATION_FAILED);
        } else {
            ORION_RENDERER_LOG_DEBUG("Using queue family {} for graphics & presentation", graphics_queue_family_index);
        }

        // Create a single graphics/presentation queue
        const auto queue_priority = 1.0f;
        const auto queue_info = VkDeviceQueueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .queueFamilyIndex = graphics_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = &queue_priority,
        };

        // Enabled device extensions
        std::vector<const char*> enabled_extensions;
        enabled_extensions.push_back("VK_KHR_swapchain");

        // Create device
        const auto vulkan_12_features = VkPhysicalDeviceVulkan12Features{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
            .pNext = nullptr,
            .timelineSemaphore = VK_TRUE,
        };
        const auto device_info = VkDeviceCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = &vulkan_12_features,
            .flags = {},
            .queueCreateInfoCount = 1,
            .pQueueCreateInfos = &queue_info,
            .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
            .ppEnabledExtensionNames = enabled_extensions.data(),
        };
        VkDevice device = VK_NULL_HANDLE;
        if (VkResult err = vkCreateDevice(physical_device, &device_info, nullptr, &device)) {
            ORION_RENDERER_LOG_ERROR("vkCreateDevice() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        } else {
            ORION_RENDERER_LOG_INFO("Created VkDevice {}", fmt::ptr(device));
            volkLoadDevice(device);
        }

        return VulkanDevice{device, physical_device, vk_instance, graphics_queue_family_index};
    }

    VulkanDevice::VulkanDevice(
        VkDevice device,
        VkPhysicalDevice physical_device,
        VkInstance instance,
        std::uint32_t graphics_queue_family)
        : vk_device(device)
        , vk_physical_device(physical_device)
        , vk_instance(instance)
        , graphics_queue_family(graphics_queue_family)
    {
    }

    VulkanDevice::VulkanDevice(VulkanDevice&& other) noexcept
        : vk_device(std::exchange(other.vk_device, VK_NULL_HANDLE))
        , vk_physical_device(other.vk_physical_device)
        , vk_instance(other.vk_instance)
        , graphics_queue_family(other.graphics_queue_family)
    {
    }

    VulkanDevice& VulkanDevice::operator=(VulkanDevice&& other) noexcept
    {
        if (this != &other) {
            vk_device = std::exchange(other.vk_device, VK_NULL_HANDLE);
            vk_physical_device = other.vk_physical_device;
            vk_instance = other.vk_instance;
            graphics_queue_family = other.graphics_queue_family;
        }
        return *this;
    }

    VulkanDevice::~VulkanDevice()
    {
        if (vk_device != VK_NULL_HANDLE) {
            vkDeviceWaitIdle(vk_device);
            vkDestroyDevice(vk_device, nullptr);
            ORION_RENDERER_LOG_INFO("Destroyed VkDevice {}", fmt::ptr(vk_device));
        }
    }

    tl::expected<VulkanSurface, VkResult> VulkanDevice::create_surface(const Window& window)
    {
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        if (VkResult err = glfwCreateWindowSurface(vk_instance, window.impl()->window, nullptr, &surface)) {
            ORION_RENDERER_LOG_ERROR("glfwCreateWindowSurface() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        } else {
            ORION_RENDERER_LOG_INFO("Created VkSurfaceKHR {}", fmt::ptr(surface));
            return VulkanSurface{vk_instance, vk_physical_device, surface};
        }
    }

    tl::expected<VulkanSwapchain, VkResult> VulkanDevice::create_swapchain(const VulkanSwapchainDesc& desc)
    {
        const auto surface_capabilities = desc.surface.surface_capabilities();
        if (!surface_capabilities) {
            return tl::unexpected(surface_capabilities.error());
        }

        // Clamp image count
        // VkSurfaceCapabilitiesKHR::maxImageCount == 0 means no driver limit
        const auto image_count = std::clamp(desc.requested_image_count,
                                            surface_capabilities->minImageCount,
                                            surface_capabilities->maxImageCount != 0 ? surface_capabilities->maxImageCount : VulkanSwapchain::max_image_count);
        ORION_RENDERER_LOG_DEBUG("Using VkSwapchainCreateInfoKHR::minImageCount = {}", image_count);

        // Set swapchain extent
        // if VkSurfaceCapabilitiesKHR::currentExtent == (0xFFFFFFFF,0xFFFFFFFF)
        //  extent = clamp(window->extent(), VkSurfaceCapabilitiesKHR::minImageExtent, VkSurfaceCapabilitiesKHR::maxImageExtent)
        // else
        //  extent = VkSurfaceCapabilitiesKHR::currentExtent
        const auto image_extent = [&]() {
            static constexpr auto undefined_extent = VkExtent2D{0xFFFFFFFF, 0xFFFFFFFF};
            if (surface_capabilities->currentExtent.width == undefined_extent.width &&
                surface_capabilities->currentExtent.height == undefined_extent.height) {

                const auto width = std::clamp(desc.requested_extent.width,
                                              surface_capabilities->minImageExtent.width,
                                              surface_capabilities->maxImageExtent.width);
                const auto height = std::clamp(desc.requested_extent.height,
                                               surface_capabilities->minImageExtent.height,
                                               surface_capabilities->maxImageExtent.height);
                return VkExtent2D{width, height};
            } else {
                return surface_capabilities->currentExtent;
            }
        }();
        ORION_RENDERER_LOG_DEBUG("Using VkSwapchainCreateInfoKHR::imageExtent = ({},{})", image_extent.width, image_extent.height);

        // Select image format
        // if requested_image_format in surface_formats
        //  image_format = requested_image_format
        // else
        //  image_format = surface_formats[0]
        static constexpr auto colorspace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
        const auto surface_formats = desc.surface.surface_formats();
        if (!surface_formats) {
            return tl::unexpected(surface_formats.error());
        }
        const auto image_format = [&]() {
            for (const auto& surface_format : *surface_formats) {
                if (surface_format.format == desc.requested_image_format && surface_format.colorSpace == colorspace) {
                    return surface_format.format;
                }
            }
            // vkGetPhysicalDeviceSurfaceFormatsKHR() guaranteed to return count >= 1
            return (*surface_formats)[0].format;
        }();
        ORION_RENDERER_LOG_DEBUG("Using VkSwapchainCreateInfoKHR::imageFormat = {}", string_VkFormat(image_format));
        ORION_RENDERER_LOG_DEBUG("Using VkSwapchainCreateInfoKHR::imageColorSpace = {}", string_VkColorSpaceKHR(colorspace));

        // Select present mode
        // if requested_present_mode in present_modes
        //  present_mode = requested_present_mode
        // else
        //  present_mode = VK_PRESENT_MODE_FIFO_KHR
        const auto present_modes = desc.surface.present_modes();
        if (!present_modes) {
            return tl::unexpected(present_modes.error());
        }
        const auto present_mode = [&]() {
            for (auto mode : *present_modes) {
                if (mode == desc.requested_present_mode) {
                    return mode;
                }
            }
            return VK_PRESENT_MODE_FIFO_KHR;
        }();
        ORION_RENDERER_LOG_DEBUG("Using VkSwapchainCreateInfoKHR::presentMode = {}", string_VkPresentModeKHR(present_mode));

        // Create swapchain
        ORION_RENDERER_LOG_DEBUG("Using VkSwapchainCreateInfoKHR::oldSwapchain = {}", fmt::ptr(desc.old_swapchain));
        const auto swapchain_info = VkSwapchainCreateInfoKHR{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
            .pNext = nullptr,
            .flags = {},
            .surface = desc.surface.vk_surface,
            .minImageCount = image_count,
            .imageFormat = image_format,
            .imageColorSpace = colorspace,
            .imageExtent = image_extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .preTransform = surface_capabilities->currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = present_mode,
            .clipped = VK_TRUE,
            .oldSwapchain = desc.old_swapchain,
        };
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        if (VkResult err = vkCreateSwapchainKHR(vk_device, &swapchain_info, nullptr, &swapchain)) {
            ORION_RENDERER_LOG_ERROR("vkCreateSwapchainKHR() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        } else {
            ORION_RENDERER_LOG_INFO("Created VkSwapchainKHR {}", fmt::ptr(swapchain));
        }
        return VulkanSwapchain{vk_device, swapchain, image_count, image_extent, image_format, present_mode};
    }

    tl::expected<VulkanCommandPool, VkResult> VulkanDevice::create_command_pool(const VulkanCommandPoolDesc& desc)
    {
        const auto command_pool_info = VkCommandPoolCreateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext = nullptr,
            .flags = desc.flags,
            .queueFamilyIndex = desc.queue_family_index,
        };
        VkCommandPool command_pool = VK_NULL_HANDLE;
        if (VkResult err = vkCreateCommandPool(vk_device, &command_pool_info, nullptr, &command_pool)) {
            ORION_RENDERER_LOG_ERROR("vkCreateCommandPool() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        } else {
            ORION_RENDERER_LOG_INFO("Created VkCommandPool {}", fmt::ptr(command_pool));
        }

        // Preallocate max command buffers
        const auto cb_allocate_info = VkCommandBufferAllocateInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .pNext = nullptr,
            .commandPool = command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = VulkanCommandPool::max_command_buffers,
        };
        std::array<VkCommandBuffer, VulkanCommandPool::max_command_buffers> command_buffers = {};
        if (VkResult err = vkAllocateCommandBuffers(vk_device, &cb_allocate_info, command_buffers.data())) {
            ORION_RENDERER_LOG_ERROR("vkAllocateCommandBuffers() failed: {}", string_VkResult(err));
            vkDestroyCommandPool(vk_device, command_pool, nullptr);
            return tl::unexpected(err);
        } else {
            ORION_RENDERER_LOG_INFO("Allocated {} VkCommandBuffer(s)", command_buffers.size());
        }

        return VulkanCommandPool{vk_device, command_pool, command_buffers};
    }

    tl::expected<VulkanSemaphore, VkResult> VulkanDevice::create_binary_semaphore()
    {
        const auto semaphore_info = VkSemaphoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
        VkSemaphore semaphore = VK_NULL_HANDLE;
        if (VkResult err = vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &semaphore)) {
            ORION_RENDERER_LOG_ERROR("vkCreateSemaphore() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        } else {
            ORION_RENDERER_LOG_INFO("Created VkSemaphore (binary) {}", fmt::ptr(semaphore));
            return VulkanSemaphore{vk_device, semaphore};
        }
    }

    tl::expected<VulkanSemaphore, VkResult> VulkanDevice::create_timeline_semaphore(std::uint64_t initial_value)
    {
        const auto semaphore_type_info = VkSemaphoreTypeCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .pNext = nullptr,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = initial_value,
        };
        const auto semaphore_info = VkSemaphoreCreateInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &semaphore_type_info,
            .flags = {},
        };
        VkSemaphore semaphore = VK_NULL_HANDLE;
        if (VkResult err = vkCreateSemaphore(vk_device, &semaphore_info, nullptr, &semaphore)) {
            ORION_RENDERER_LOG_ERROR("vkCreateSemaphore() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        } else {
            ORION_RENDERER_LOG_INFO("Created VkSemaphore (timeline) {}", fmt::ptr(semaphore));
            return VulkanSemaphore{vk_device, semaphore};
        }
    }

    VulkanSurface::VulkanSurface(VkInstance instance, VkPhysicalDevice physical_device, VkSurfaceKHR surface)
        : vk_instance(instance)
        , vk_physical_device(physical_device)
        , vk_surface(surface)
    {
    }

    VulkanSurface::VulkanSurface(VulkanSurface&& other) noexcept
        : vk_instance(other.vk_instance)
        , vk_physical_device(other.vk_physical_device)
        , vk_surface(std::exchange(other.vk_surface, VK_NULL_HANDLE))
    {
    }

    VulkanSurface& VulkanSurface::operator=(VulkanSurface&& other) noexcept
    {
        if (this != &other) {
            vk_surface = std::exchange(other.vk_surface, VK_NULL_HANDLE);
            vk_instance = other.vk_instance;
            vk_physical_device = other.vk_physical_device;
        }
        return *this;
    }

    VulkanSurface::~VulkanSurface()
    {
        if (vk_surface != VK_NULL_HANDLE) {
            vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr);
            ORION_RENDERER_LOG_INFO("Destroyed VkSurfaceKHR {}", fmt::ptr(vk_surface));
        }
    }

    tl::expected<VkSurfaceCapabilitiesKHR, VkResult> VulkanSurface::surface_capabilities() const
    {
        VkSurfaceCapabilitiesKHR surface_capabilities;
        if (VkResult err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, vk_surface, &surface_capabilities)) {
            ORION_RENDERER_LOG_ERROR("vkGetPhysicalDeviceSurfaceCapabilitiesKHR() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        } else {
            return surface_capabilities;
        }
    }

    tl::expected<std::vector<VkSurfaceFormatKHR>, VkResult> VulkanSurface::surface_formats() const
    {
        std::uint32_t surface_format_count = 0;
        if (VkResult err = vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &surface_format_count, nullptr)) {
            ORION_RENDERER_LOG_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        }
        std::vector<VkSurfaceFormatKHR> surface_formats(surface_format_count);
        if (VkResult err = vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &surface_format_count, surface_formats.data())) {
            ORION_RENDERER_LOG_ERROR("vkGetPhysicalDeviceSurfaceFormatsKHR() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        }
        return surface_formats;
    }

    tl::expected<std::vector<VkPresentModeKHR>, VkResult> VulkanSurface::present_modes() const
    {
        std::uint32_t present_mode_count = 0;
        if (VkResult err = vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &present_mode_count, nullptr)) {
            ORION_RENDERER_LOG_ERROR("vkGetPhysicalDeviceSurfacePresentModesKHR() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        }
        std::vector<VkPresentModeKHR> present_modes(present_mode_count);
        if (VkResult err = vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &present_mode_count, present_modes.data())) {
            ORION_RENDERER_LOG_ERROR("vkGetPhysicalDeviceSurfacePresentModesKHR() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        }
        return present_modes;
    }

    VulkanSwapchain::VulkanSwapchain(
        VkDevice device,
        VkSwapchainKHR swapchain,
        std::uint32_t image_count,
        VkExtent2D image_extent,
        VkFormat image_format,
        VkPresentModeKHR present_mode)
        : vk_device(device)
        , vk_swapchain(swapchain)
        , image_count(image_count)
        , image_extent(image_extent)
        , image_format(image_format)
        , present_mode(present_mode)
    {
    }

    VulkanSwapchain::VulkanSwapchain(VulkanSwapchain&& other) noexcept
        : vk_device(other.vk_device)
        , vk_swapchain(std::exchange(other.vk_swapchain, VK_NULL_HANDLE))
        , image_count(std::exchange(other.image_count, 0))
        , image_extent(other.image_extent)
        , image_format(other.image_format)
        , present_mode(other.present_mode)
    {
    }

    VulkanSwapchain& VulkanSwapchain::operator=(VulkanSwapchain&& other) noexcept
    {
        if (this != &other) {
            vk_device = other.vk_device;
            vk_swapchain = std::exchange(other.vk_swapchain, VK_NULL_HANDLE);
            image_count = std::exchange(other.image_count, 0);
            image_extent = other.image_extent;
            image_format = other.image_format;
            present_mode = other.present_mode;
        }
        return *this;
    }

    VulkanSwapchain::~VulkanSwapchain()
    {
        if (vk_swapchain != VK_NULL_HANDLE) {
            vkDestroySwapchainKHR(vk_device, vk_swapchain, nullptr);
            ORION_RENDERER_LOG_INFO("Destroyed VkSwapchainKHR {}", fmt::ptr(vk_swapchain));
        }
    }

    VulkanCommandPool::VulkanCommandPool(
        VkDevice device,
        VkCommandPool command_pool,
        std::array<VkCommandBuffer, max_command_buffers> command_buffers)
        : vk_device(device)
        , vk_command_pool(command_pool)
        , vk_command_buffers(command_buffers)
    {
    }

    VulkanCommandPool::VulkanCommandPool(VulkanCommandPool&& other) noexcept
        : vk_device(other.vk_device)
        , vk_command_pool(std::exchange(other.vk_command_pool, VK_NULL_HANDLE))
        , vk_command_buffers(std::exchange(other.vk_command_buffers, {}))
    {
    }

    VulkanCommandPool& VulkanCommandPool::operator=(VulkanCommandPool&& other) noexcept
    {
        if (this != &other) {
            vk_device = other.vk_device;
            vk_command_pool = std::exchange(other.vk_command_pool, VK_NULL_HANDLE);
            vk_command_buffers = std::exchange(other.vk_command_buffers, {});
        }
        return *this;
    }

    VulkanCommandPool::~VulkanCommandPool()
    {
        if (vk_command_pool != VK_NULL_HANDLE) {
            vkFreeCommandBuffers(vk_device, vk_command_pool, max_command_buffers, vk_command_buffers.data());
            vkDestroyCommandPool(vk_device, vk_command_pool, nullptr);
            ORION_RENDERER_LOG_INFO("Destroyed VkCommandPool {}", fmt::ptr(vk_command_pool));
        }
    }

    VulkanSemaphore::VulkanSemaphore(VkDevice device, VkSemaphore semaphore)
        : vk_device(device)
        , vk_semaphore(semaphore)
    {
    }

    VulkanSemaphore::VulkanSemaphore(VulkanSemaphore&& other) noexcept
        : vk_device(other.vk_device)
        , vk_semaphore(std::exchange(other.vk_semaphore, VK_NULL_HANDLE))
    {
    }

    VulkanSemaphore& VulkanSemaphore::operator=(VulkanSemaphore&& other) noexcept
    {
        if (this != &other) {
            vk_device = other.vk_device;
            vk_semaphore = std::exchange(other.vk_semaphore, VK_NULL_HANDLE);
        }
        return *this;
    }

    VulkanSemaphore::~VulkanSemaphore()
    {
        if (vk_semaphore != VK_NULL_HANDLE) {
            vkDestroySemaphore(vk_device, vk_semaphore, nullptr);
            ORION_RENDERER_LOG_INFO("Destroyed VkSemaphore {}", fmt::ptr(vk_semaphore));
        }
    }
} // namespace orion
