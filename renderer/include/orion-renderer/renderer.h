#pragma once

#include "orion-core/config.h"
#include "orion-core/platform.h"
#include "orion-renderapi/render_backend.h"
#include "orion-renderer/config.h"
#include "shader_compiler.h"

#include <memory>
#include <span>
#include <spdlog/logger.h>

namespace orion
{
    using pfnSelectPhysicalDevice =
        std::uint32_t (*)(std::span<const PhysicalDeviceDesc>);

    std::uint32_t select_discrete(std::span<const PhysicalDeviceDesc>);

    const char* default_backend_module(Platform platform = current_platform);

    struct RendererDesc {
        const char* backend_module = default_backend_module();
        pfnSelectPhysicalDevice device_select_fn = nullptr;
    };

    class Renderer
    {
    public:
        explicit Renderer(const RendererDesc& desc);

        [[nodiscard]] auto backend() const noexcept { return render_backend_.get(); }
        [[nodiscard]] auto device() const noexcept { return render_device_.get(); }

        void begin_frame();
        void end_frame();
        void present();

        static spdlog::logger* logger();

    private:
        static std::unique_ptr<RenderBackend> create_backend(const Module& backend_module);
        static std::unique_ptr<RenderDevice> create_device(RenderBackend* backend, pfnSelectPhysicalDevice device_select_fn);

        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
    };
} // namespace orion
