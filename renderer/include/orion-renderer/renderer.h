#pragma once

#include "orion-renderapi/render_backend.h"
#include "orion-renderer/config.h"

#ifndef ORION_RENDERER_LOG_LEVEL
    #define ORION_RENDERER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include <memory>
#include <span>
#include <spdlog/logger.h>

namespace orion
{
    using pfnSelectPhysicalDevice =
        std::uint32_t (*)(std::span<const PhysicalDeviceDesc>);

    std::uint32_t select_discrete(std::span<const PhysicalDeviceDesc>);

    class Renderer
    {
    public:
        explicit Renderer(const char* backend_module,
                          pfnSelectPhysicalDevice device_select_fn = nullptr);

        [[nodiscard]] auto device() const noexcept { return render_device_.get(); }

        static spdlog::logger* logger();

    private:
        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
    };
} // namespace orion
