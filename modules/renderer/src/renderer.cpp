#include "orion-renderer/renderer.h"

#include <spdlog/spdlog.h> // SPDLOG_*

namespace orion
{
    Renderer::Renderer(const char* backend_module)
        : backend_module_(backend_module)
    {
        // Load the factory function
        auto pfnCreateBackend = backend_module_.load_symbol<RenderBackend*(void)>("create_render_backend");
        SPDLOG_TRACE("Loaded create_render_backend() (at: {})", fmt::ptr(pfnCreateBackend));

        // Create the backend
        render_backend_ = std::unique_ptr<RenderBackend>(pfnCreateBackend());
        SPDLOG_DEBUG("Render backend \"{}\" initialized", render_backend_->name());
    }
} // namespace orion
