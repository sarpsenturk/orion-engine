#pragma once

#include "orion-renderer/config.h"
#include "shader_compiler.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_device.h"

#include "orion-core/config.h"
#include "orion-core/platform.h"

#include <memory>
#include <span>
#include <spdlog/logger.h>

namespace orion
{
    const char* default_backend_module(Platform platform = current_platform);

    struct RendererDesc {
        const char* backend_module = default_backend_module();
        pfnSelectPhysicalDevice device_select_fn = nullptr;
    };

    class Renderer
    {
    public:
        static constexpr auto frames_in_flight = ORION_FRAMES_IN_FLIGHT;

        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto backend() const noexcept { return render_backend_.get(); }
        [[nodiscard]] auto device() const noexcept { return render_device_.get(); }

        static spdlog::logger* logger();

    private:
        std::unique_ptr<RenderBackend> create_backend(const Module& backend_module) const;
        std::unique_ptr<RenderDevice> create_device(RenderBackend* backend, pfnSelectPhysicalDevice device_select_fn) const;

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
    };
} // namespace orion
