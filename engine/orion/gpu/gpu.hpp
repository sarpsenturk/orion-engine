#pragma once

#include <cstdint>

namespace orion
{
    namespace gpu
    {
        struct Device;
        struct CommandAllocator;
        struct CommandBuffer;

        enum class Queue {
            Graphics = 0,
            Compute,
            Transfer,
        };

        struct DeviceDesc {
            std::uint32_t device_index = 0u;
        };

        Device* create_device(const DeviceDesc& desc);
        void destroy_device(Device* device);

        CommandAllocator* create_command_allocator(Device* device, Queue queue);
        CommandBuffer* begin_command_buffer(Device* device, CommandAllocator* allocator);
        void reset_command_allocator(Device* device, CommandAllocator* allocator);
        void destroy_command_allocator(Device* device, CommandAllocator* allocator);

        [[nodiscard]] const char* backend_name();
        [[nodiscard]] const char* backend_version();
        [[nodiscard]] const char* device_name(const Device* device);
    } // namespace gpu
} // namespace orion
