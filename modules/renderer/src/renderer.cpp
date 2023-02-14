#include "orion-renderer/renderer.h"

#include <spdlog/spdlog.h> // SPDLOG_*

namespace orion
{
    Renderer::Renderer(const char* backend_module)
    {
        // Load the backend module
        backend_module_ = Module(backend_module);

        // Load the factory function
        auto pfnCreateBackend = backend_module_.load_symbol<RenderBackend*(void)>("create_render_backend");
        SPDLOG_TRACE("Loaded create_render_backend() (at: {})", fmt::ptr(pfnCreateBackend));

        // Create the backend
        render_backend_ = std::unique_ptr<RenderBackend>(pfnCreateBackend());
        if (!render_backend_) {
            throw std::runtime_error("Failed to create render backend");
        }

        SPDLOG_DEBUG("Render backend \"{}\" initialized", render_backend_->name());
    }
} // namespace orion
