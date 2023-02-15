#include "orion-renderapi/render_backend.h"

#include <spdlog/spdlog.h> // SPDLOG_*

namespace orion
{
    std::vector<PhysicalDeviceDesc> RenderBackend::enumerate_physical_devices()
    {
        auto devices = enumerate_physical_devices_api();
        SPDLOG_DEBUG("Found {} physical devices", devices.size());
        return devices;
    }
} // namespace orion
