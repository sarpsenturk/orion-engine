#pragma once

#include "orion-renderer/config.h"

#include "orion-renderapi/render_backend.h"
#include "orion-renderapi/render_device.h"

#include "orion-core/platform.h"

#include <memory>
#include <spdlog/logger.h>

namespace orion
{
    const char* default_backend_module(Platform platform = k_current_platform);

    struct RendererDesc {
        const char* backend_module = default_backend_module();
        pfnSelectPhysicalDevice device_select_fn = nullptr;
    };

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto* backend() const noexcept { return render_backend_.get(); }
        [[nodiscard]] auto* device() const noexcept { return render_device_.get(); }

    private:
        static spdlog::logger* logger();

        std::unique_ptr<RenderBackend> create_render_backend() const;
        std::unique_ptr<RenderDevice> create_render_device(pfnSelectPhysicalDevice device_select_fn) const;

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
    };
} // namespace orion
