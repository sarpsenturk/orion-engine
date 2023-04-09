#pragma once

#include "orion-core/dl_lib.h"
#include "orion-renderapi/types.h"
#include "render_device.h"

#include <memory> // std::unique_ptr
#include <vector> // std::vector

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

    class RenderBackend
    {
    public:
        RenderBackend() = default;
        virtual ~RenderBackend() = default;

        [[nodiscard]] std::vector<PhysicalDeviceDesc> enumerate_physical_devices();
        [[nodiscard]] std::unique_ptr<RenderDevice> create_device(std::uint32_t physical_device_index);

        [[nodiscard]] virtual const char* name() const noexcept = 0;

    protected:
        RenderBackend(const RenderBackend&) = default;
        RenderBackend(RenderBackend&&) noexcept = default;
        RenderBackend& operator=(const RenderBackend&) = default;
        RenderBackend& operator=(RenderBackend&&) noexcept = default;

    private:
        [[nodiscard]] virtual std::vector<PhysicalDeviceDesc> enumerate_physical_devices_api() = 0;
        [[nodiscard]] virtual std::unique_ptr<RenderDevice> create_device_api(std::uint32_t physical_device_index) = 0;
    };
} // namespace orion

extern "C" ORION_RENDER_API orion::RenderBackend* create_render_backend();
