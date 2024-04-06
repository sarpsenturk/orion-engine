#include "orion-renderapi/render_backend.h"

#include <algorithm>

#ifndef ORION_RENDERAPI_LOG_LEVEL
    #define ORION_RENDERAPI_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h> // SPDLOG_*

namespace orion
{
    physical_device_index_t device_select_type(std::span<const PhysicalDeviceDesc> devices, PhysicalDeviceType type)
    {
        if (auto iter = std::ranges::find_if(devices, [type](const auto& device) { return device.type == type; });
            iter != devices.end()) {
            return iter->index;
        }
        return invalid_physical_device_index;
    }

    physical_device_index_t device_select_integrated(std::span<const PhysicalDeviceDesc> devices)
    {
        return device_select_type(devices, PhysicalDeviceType::Integrated);
    }

    physical_device_index_t device_select_discrete(std::span<const PhysicalDeviceDesc> devices)
    {
        return device_select_type(devices, PhysicalDeviceType::Discrete);
    }

    physical_device_index_t device_select_virtual(std::span<const PhysicalDeviceDesc> devices)
    {
        return device_select_type(devices, PhysicalDeviceType::Virtual);
    }

    physical_device_index_t device_select_cpu(std::span<const PhysicalDeviceDesc> devices)
    {
        return device_select_type(devices, PhysicalDeviceType::CPU);
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

    std::unique_ptr<ShaderReflector> RenderBackend::create_shader_reflector()
    {
        return create_shader_reflector_api();
    }
} // namespace orion
