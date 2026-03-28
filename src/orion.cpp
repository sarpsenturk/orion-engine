#include "orion/orion.hpp"

#include "orion/config.h"
#include "orion/platform.hpp"

#include <exception>

namespace orion
{
    tl::expected<Engine, std::string> Engine::initialize()
    {
        // Initialize logger
        auto logger = Logger::initialize();
        if (!logger) {
            return tl::unexpected(std::move(logger.error()));
        }

        // Initialize main window
        auto window = Window::initialize({.title = "Orion", .width = 800, .height = 600});
        if (!window) {
            return tl::unexpected(std::move(window.error()));
        }

        return Engine{std::move(*logger), std::move(*window)};
    }

    Engine::Engine(Logger logger, Window window)
        : logger_(std::move(logger))
        , window_(std::move(window))
    {
        ORION_CORE_LOG_INFO("Orion Engine v{} {}-{}-{}-{}",
                            ORION_VERSION, ORION_BUILD_TYPE, ORION_ARCH_NAME, ORION_COMPILER_NAME, ORION_PLATFORM_NAME);
    }

    void Engine::run(std::unique_ptr<Application> app)
    {
        try {
            while (!app->should_exit()) {
                window_.poll_events();
                app->update();
                app->render();
            }
        } catch (const std::exception& ex) {
            ORION_CORE_LOG_ERROR("Fatal exception: {}", ex.what());
        } catch (...) {
            ORION_CORE_LOG_ERROR("Fatal exception: unknown");
        }
    }
} // namespace orion
