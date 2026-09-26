#pragma once

#include <cstdint>

namespace orion
{
    namespace gpu
    {
        struct Device;
        struct Swapchain;
        struct CommandBuffer;
        struct CommandAllocator;

        enum class Queue {
            Graphics = 0,
            Compute,
            Transfer,
        };

        enum class Format {
            Undefined = 0,
            BGRA8_Unorm,
        };

        struct TimelinePoint {
            Queue queue         = Queue::Graphics;
            std::uint64_t value = 0ull;
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

        CommandAllocator* create_command_allocator(Device* device, Queue queue);
        CommandBuffer* begin_command_buffer(Device* device, CommandAllocator* allocator);
        void reset_command_allocator(Device* device, CommandAllocator* allocator);
        void destroy_command_allocator(Device* device, CommandAllocator* allocator);

        void wait_timeline(Device* device, const TimelinePoint& timeline_point);
        void wait_idle(Device* device);

        [[nodiscard]] const char* backend_name();
        [[nodiscard]] const char* backend_version();
        [[nodiscard]] const char* device_name(const Device* device);
    } // namespace gpu
} // namespace orion

