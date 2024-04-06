#pragma once

#include "orion-core/dyn_lib.h"
#include "orion-renderapi/defs.h"
#include "orion-renderapi/render_device.h"
#include "orion-renderapi/shader_reflection.h"

#include <memory>
#include <vector>

#ifdef ORION_RENDER_BACKEND_EXPORT
    #define ORION_RENDER_API ORION_EXPORT
#else
    #define ORION_RENDER_API ORION_IMPORT
#endif

namespace spdlog
{
    class logger;
}

namespace orion
{
    physical_device_index_t device_select_type(std::span<const PhysicalDeviceDesc> devices, PhysicalDeviceType type);
    physical_device_index_t device_select_integrated(std::span<const PhysicalDeviceDesc> devices);
    physical_device_index_t device_select_discrete(std::span<const PhysicalDeviceDesc> devices);
    physical_device_index_t device_select_virtual(std::span<const PhysicalDeviceDesc> devices);
    physical_device_index_t device_select_cpu(std::span<const PhysicalDeviceDesc> devices);

    class RenderBackend
    {
    public:
        explicit RenderBackend(const char* logger_name = "orion-renderapi");
        virtual ~RenderBackend() = default;

        [[nodiscard]] std::vector<PhysicalDeviceDesc> enumerate_physical_devices();
        [[nodiscard]] std::unique_ptr<RenderDevice> create_device(std::uint32_t physical_device_index);
        [[nodiscard]] std::unique_ptr<ShaderReflector> create_shader_reflector();

        [[nodiscard]] auto logger() const noexcept { return logger_.get(); }

        [[nodiscard]] virtual const char* name() const noexcept = 0;

    protected:
        RenderBackend(const RenderBackend&) = default;
        RenderBackend(RenderBackend&&) noexcept = default;
        RenderBackend& operator=(const RenderBackend&) = default;
        RenderBackend& operator=(RenderBackend&&) noexcept = default;

    private:
        [[nodiscard]] virtual std::vector<PhysicalDeviceDesc> enumerate_physical_devices_api() = 0;
        [[nodiscard]] virtual std::unique_ptr<RenderDevice> create_device_api(std::uint32_t physical_device_index) = 0;
        [[nodiscard]] virtual std::unique_ptr<ShaderReflector> create_shader_reflector_api() = 0;

        std::shared_ptr<spdlog::logger> logger_;
    };
} // namespace orion

extern "C" ORION_RENDER_API orion::RenderBackend* create_orion_render_backend();
using CreateOrionRenderBackendFn = decltype(create_orion_render_backend);
