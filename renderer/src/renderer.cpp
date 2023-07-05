#include "orion-renderer/renderer.h"

#include "orion-utils/assertion.h"

#include <algorithm>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace orion
{
    namespace
    {
        std::uint32_t select_device_type(
            std::span<const PhysicalDeviceDesc> physical_devices,
            PhysicalDeviceType expected)
        {
            if (auto iter = std::ranges::find_if(physical_devices, check_device_type(expected));
                iter != physical_devices.end()) {
                return iter->index;
            }
            return UINT32_MAX;
        }
    } // namespace

    std::uint32_t select_discrete(std::span<const PhysicalDeviceDesc> physical_devices)
    {
        return select_device_type(physical_devices, PhysicalDeviceType::Discrete);
    }

    Renderer::Renderer(const char* backend_module,
                       pfnSelectPhysicalDevice device_select_fn)
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
            SPDLOG_LOGGER_DEBUG(logger(), "-- Index: {}", physical_device.index);
            SPDLOG_LOGGER_DEBUG(logger(), "-- Type: {}", physical_device.type);
        }

        // Select the physical device to use
        const auto physical_device_index =
            [device_select_fn, &physical_devices]() -> std::uint32_t {
            if (device_select_fn) {
                return device_select_fn(physical_devices);
            }
            return 0;
        }();
        if (physical_device_index == UINT32_MAX) {
            throw std::runtime_error("Couldn't find a suitable physical device");
        }

        SPDLOG_LOGGER_INFO(logger(), "Using physical device index {}", physical_device_index);

        // Create the render device
        render_device_ = render_backend_->create_device(physical_device_index);

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
