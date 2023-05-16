#include "orion-renderapi/render_backend.h"

#include <spdlog/sinks/stdout_color_sinks.h> // spdlog::stdout_color_mt
#include <spdlog/spdlog.h>                   // SPDLOG_*

namespace orion
{
    RenderBackend::RenderBackend(const char* logger_name)
        : logger_(spdlog::stdout_color_mt(logger_name))
    {
        logger_->set_pattern("[%n] [%^%l%$] %v");
        logger_->set_level(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
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
