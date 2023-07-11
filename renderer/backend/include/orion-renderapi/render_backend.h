#pragma once

#include "orion-core/dl_lib.h"
#include "orion-renderapi/types.h"
#include "render_device.h"

#include <memory>          // std::unique_ptr
#include <spdlog/logger.h> // spdlog::logger
#include <vector>          // std::vector

#ifndef ORION_RENDERAPI_LOG_LEVEL
    #define ORION_RENDERAPI_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#ifdef ORION_RENDER_BACKEND_EXPORT
    #define ORION_RENDER_API ORION_EXPORT
#else
    #define ORION_RENDER_API ORION_IMPORT
#endif

namespace orion
{
    struct PhysicalDeviceDesc {
        std::uint32_t index;
        PhysicalDeviceType type;
        std::string name;
    };

    inline constexpr auto check_device_type(PhysicalDeviceType type)
    {
        return [type](const PhysicalDeviceDesc& desc) { return desc.type == type; };
    }

    class RenderBackend
    {
    public:
        RenderBackend(const char* logger_name = "orion-renderapi");
        virtual ~RenderBackend() = default;

        [[nodiscard]] std::vector<PhysicalDeviceDesc> enumerate_physical_devices();
        [[nodiscard]] std::unique_ptr<RenderDevice> create_device(std::uint32_t physical_device_index);

        [[nodiscard]] auto logger() const noexcept { return logger_.get(); }

        [[nodiscard]] virtual const char* name() const noexcept = 0;
        [[nodiscard]] virtual ShaderObjectType shader_object_type() const noexcept = 0;

    protected:
        RenderBackend(const RenderBackend&) = default;
        RenderBackend(RenderBackend&&) noexcept = default;
        RenderBackend& operator=(const RenderBackend&) = default;
        RenderBackend& operator=(RenderBackend&&) noexcept = default;

    private:
        [[nodiscard]] virtual std::vector<PhysicalDeviceDesc> enumerate_physical_devices_api() = 0;
        [[nodiscard]] virtual std::unique_ptr<RenderDevice> create_device_api(std::uint32_t physical_device_index) = 0;

        std::shared_ptr<spdlog::logger> logger_;
    };
} // namespace orion

extern "C" ORION_RENDER_API orion::RenderBackend* create_render_backend();
