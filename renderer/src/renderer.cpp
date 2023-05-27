#include "orion-renderer/renderer.h"

#include <spdlog/sinks/stdout_color_sinks.h> // spdlog::stdout_color_st
#include <spdlog/spdlog.h>                   // SPDLOG_LOGGER_*

namespace orion
{
    Renderer::Renderer(const char* backend_module)
    {
        // Load the backend module
        backend_module_ = Module(backend_module);

        // Load the factory function
        auto pfnCreateBackend = backend_module_.load_symbol<RenderBackend*(void)>("create_render_backend");
        SPDLOG_LOGGER_TRACE(logger(), "Loaded create_render_backend() (at: {})", fmt::ptr(pfnCreateBackend));

        // Create the backend
        render_backend_ = std::unique_ptr<RenderBackend>(pfnCreateBackend());
        if (!render_backend_) {
            throw std::runtime_error("Failed to create render backend");
        }

        // Get the physical devices
        auto physical_devices = render_backend_->enumerate_physical_devices();
        SPDLOG_LOGGER_DEBUG(logger(), "Found {} physical device(s):", physical_devices.size());
        for (const auto& physical_device : physical_devices) {
            SPDLOG_LOGGER_DEBUG(logger(), "{}", physical_device.name);
            SPDLOG_LOGGER_DEBUG(logger(), "-- Type: {}", to_string(physical_device.type));
        }

        // Select the physical device to use
        auto& selected_physical_device = physical_devices[0]; // TODO: Allow the user to select

        // Create the render device
        render_device_ = render_backend_->create_device(selected_physical_device.index);

        SPDLOG_LOGGER_DEBUG(logger(), "Render backend \"{}\" initialized", render_backend_->name());
    }

    spdlog::logger* Renderer::logger()
    {
        static const auto renderer_logger = []() {
            auto logger = spdlog::stdout_color_mt("orion-renderer");
            logger->set_pattern("[%n] [%^%l%$] %v");
            logger->set_level(static_cast<spdlog::level::level_enum>(ORION_RENDERER_LOG_LEVEL));
            return logger;
        }();
        return renderer_logger.get();
    }
} // namespace orion
