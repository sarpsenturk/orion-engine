#include "orion-renderapi/render_backend.h"

#include <algorithm>

#ifndef ORION_RENDERAPI_LOG_LEVEL
    #define ORION_RENDERAPI_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h> // SPDLOG_*

namespace orion
{
    namespace
    {
        auto device_select_type_fn(PhysicalDeviceType type)
        {
            return [type](const auto& device) { return device.type == type; };
        }
    } // namespace

    std::uint32_t device_select_integrated(std::span<const PhysicalDeviceDesc> devices)
    {
        if (auto iter = std::ranges::find_if(devices, device_select_type_fn(PhysicalDeviceType::Integrated));
            iter != devices.end()) {
            return iter->index;
        }
        return invalid_physical_device_index;
    }

    std::uint32_t device_select_discrete(std::span<const PhysicalDeviceDesc> devices)
    {
        if (auto iter = std::ranges::find_if(devices, device_select_type_fn(PhysicalDeviceType::Discrete));
            iter != devices.end()) {
            return iter->index;
        }
        return invalid_physical_device_index;
    }

    RenderBackend::RenderBackend(const char* logger_name)
        : logger_(create_logger(logger_name, ORION_RENDERAPI_LOG_LEVEL))
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
