#pragma once

#include <cstdint>

namespace orion
{
    namespace gpu
    {
        struct Device;
        struct Swapchain;

        enum class Format {
            Undefined   = 0,
            BGRA8_Unorm = 44,
        };

        struct DeviceDesc {
            std::uint32_t device_idx = 0u;
        };

        struct SwapchainDesc {
            void* native_window_handle = nullptr;
            std::uint32_t width        = 0u;
            std::uint32_t height       = 0u;
            std::uint32_t image_count  = 0u;
            Format image_format        = Format::Undefined;
        };

        Device* create_device(const DeviceDesc& desc);
        void destroy_device(Device* device);

        Swapchain* create_swapchain(Device* device, const SwapchainDesc& desc);
        void destroy_swapchain(Device* device, Swapchain* swapchain);

        [[nodiscard]] const char* backend_name();
        [[nodiscard]] const char* backend_version();
        [[nodiscard]] const char* device_name(const Device* device);
    } // namespace gpu
} // namespace orion
