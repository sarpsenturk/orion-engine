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

        // Get the physical devices
        auto physical_devices = render_backend_->enumerate_physical_devices();
        SPDLOG_DEBUG("Found {} physical devices:", physical_devices.size());
        for (const auto& physical_device : physical_devices) {
            SPDLOG_DEBUG("{}", physical_device.name);
            SPDLOG_DEBUG("-- Type: {}", to_string(physical_device.type));
        }

        // Select the physical device to use
        auto& selected_physical_device = physical_devices[0]; // TODO: Allow the user to select

        // Create the render device
        render_device_ = render_backend_->create_device(selected_physical_device.index);

        // Create the render context
        render_context_ = render_device_->create_render_context();

        SPDLOG_DEBUG("Render backend \"{}\" initialized", render_backend_->name());
    }
} // namespace orion
