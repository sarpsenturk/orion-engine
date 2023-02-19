#include "orion-renderapi/render_backend.h"

#include <spdlog/spdlog.h> // SPDLOG_*

namespace orion
{
    std::vector<PhysicalDeviceDesc> RenderBackend::enumerate_physical_devices()
    {
        return enumerate_physical_devices_api();
    }

    std::unique_ptr<RenderDevice> RenderBackend::create_device(std::uint32_t physical_device_index)
    {
        auto device = create_device_api(physical_device_index);
        SPDLOG_DEBUG("Render device created");
        return device;
    }
} // namespace orion
