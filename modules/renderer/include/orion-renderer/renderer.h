#pragma once

#include "orion-renderapi/render_backend.h"
#include "orion-renderer/config.h"

#ifndef ORION_RENDERER_LOG_LEVEL
    #define ORION_RENDERER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include <memory>          // std::unique_ptr
#include <spdlog/logger.h> // spdlog::logger

namespace orion
{
    class Renderer
    {
    public:
        explicit Renderer(const char* backend_module);

        [[nodiscard]] auto device() const noexcept { return render_device_.get(); }

        static spdlog::logger* logger();

    private:
        Module backend_module_;
        std::unique_ptr<RenderBackend> render_backend_;
        std::unique_ptr<RenderDevice> render_device_;
    };
} // namespace orion
