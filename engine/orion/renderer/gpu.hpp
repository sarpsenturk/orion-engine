#pragma once

#include <cstdint>

namespace orion
{
    namespace gpu
    {
        struct Device;

        struct DeviceDesc {
            std::uint32_t device_idx = 0u;
        };

        Device* create_device(const DeviceDesc& desc);
        void destroy_device(Device* device);

        [[nodiscard]] const char* backend_name();
        [[nodiscard]] const char* backend_version();
        [[nodiscard]] const char* device_name(const Device* device);
    } // namespace gpu
} // namespace orion
