#pragma once

#include <cstdint>
#include <optional>

namespace orion
{
    namespace gpu
    {
        struct Device;
        struct Image;
        struct ImageView;
        struct CommandAllocator;
        struct CommandBuffer;
        struct Swapchain;

        enum class Queue {
            Graphics = 0,
            Compute,
            Transfer,
        };

        enum class Format {
            Undefined = 0,

            RGBA8_Unorm,
            RGBA8_Srgb,

            BGRA8_Unorm,
            BGRA8_Srgb,
        };

        struct SwapchainFrame {
            Swapchain* swapchain  = nullptr;
            Image* image          = nullptr;
            ImageView* image_view = nullptr;

            std::uint32_t image_index   = 0u;
            std::uint32_t acquire_index = 0u;
        };

        struct DeviceDesc {
            std::uint32_t device_index = 0u;
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
        std::optional<SwapchainFrame> acquire_swapchain_frame(Device* device, Swapchain* swapchain);
        void destroy_swapchain(Device* device, Swapchain* swapchain);

        CommandAllocator* create_command_allocator(Device* device, Queue queue);
        CommandBuffer* begin_command_buffer(Device* device, CommandAllocator* allocator);
        void reset_command_allocator(Device* device, CommandAllocator* allocator);
        void destroy_command_allocator(Device* device, CommandAllocator* allocator);

        [[nodiscard]] const char* backend_name();
        [[nodiscard]] const char* backend_version();
        [[nodiscard]] const char* device_name(const Device* device);
    } // namespace gpu
} // namespace orion
