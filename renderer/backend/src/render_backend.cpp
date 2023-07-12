#include "orion-renderapi/render_backend.h"

#ifndef ORION_RENDERAPI_LOG_LEVEL
    #define ORION_RENDERAPI_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include "orion-core/log.h"
#include <spdlog/spdlog.h> // SPDLOG_*

namespace orion
{
    RenderBackend::RenderBackend(const char* logger_name)
        : logger_(create_logger(logger_name, static_cast<spdlog::level::level_enum>(ORION_RENDERAPI_LOG_LEVEL)))
    {
    }

    std::vector<PhysicalDeviceDesc> RenderBackend::enumerate_physical_devices()
    {
        return enumerate_physical_devices_api();
    }

    std::unique_ptr<RenderDevice> RenderBackend::create_device(std::uint32_t physical_device_index)
    {
        auto device = create_device_api(physical_device_index);
        return device;
    }
} // namespace orion
