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
                update(*app);
                render(*app);
            }
        } catch (const std::exception& ex) {
            ORION_CORE_LOG_ERROR("Fatal exception: {}", ex.what());
        } catch (...) {
            ORION_CORE_LOG_ERROR("Fatal exception: unknown");
        }
    }

    void Engine::update(Application& app)
    {
        window_.poll_events();
        app.update();
    }

    void Engine::render(Application& app)
    {
        if (renderer_.swapchain_out_of_date()) {
            const auto width = window_.width();
            const auto height = window_.height();
            // Do not recreate swapchain or render with (0,0) extent
            if (width == 0 || height == 0) {
                return;
            }
            auto result = renderer_.recreate_swapchain(width, height);
            if (!result) {
                throw std::runtime_error(std::move(result.error()));
            }
        }

        renderer_.new_frame();
        app.render();
        renderer_.render();
    }
} // namespace orion
