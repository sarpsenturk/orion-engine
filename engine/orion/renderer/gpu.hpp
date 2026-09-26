#pragma once

namespace orion
{
    namespace gpu
    {
        struct Device;

        struct DeviceDesc {
        };

        Device* create_device(const DeviceDesc& desc);
        void destroy_device(Device* device);

        [[nodiscard]] const char* backend_name();
        [[nodiscard]] const char* backend_version();
    } // namespace gpu
} // namespace orion
