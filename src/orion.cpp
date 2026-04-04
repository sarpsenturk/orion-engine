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

        // Initialize renderer
        auto renderer = Renderer::initialize({.window = *window});
        if (!renderer) {
            return tl::unexpected(std::move(renderer.error()));
        }
        return Engine{std::move(*logger), std::move(*window), std::move(*renderer)};
    }

    Engine::Engine(Logger logger, Window window, Renderer renderer)
        : logger_(std::move(logger))
        , window_(std::move(window))
        , renderer_(std::move(renderer))
    {
        ORION_CORE_LOG_INFO("Orion Engine v{} {}-{}-{}-{}", ORION_VERSION, ORION_BUILD_TYPE, ORION_ARCH_NAME, ORION_COMPILER_NAME, ORION_PLATFORM_NAME);
    }

    void Engine::run(std::unique_ptr<Application> app)
    {
        try {
            while (!app->should_exit()) {
                window_.poll_events();
                app->update();

                renderer_.new_frame();
                app->render();
                renderer_.render();
            }
        } catch (const std::exception& ex) {
            ORION_CORE_LOG_ERROR("Fatal exception: {}", ex.what());
        } catch (...) {
            ORION_CORE_LOG_ERROR("Fatal exception: unknown");
        }
    }
} // namespace orion
