#include "orion/orion.hpp"

#include "orion/log.hpp"

#include "orion/platform/platform.hpp"
#include "orion/platform/time.hpp"
#include "orion/platform/window.hpp"

#include "orion/renderer/renderer.hpp"

#include <stdexcept>

namespace orion
{
    Engine::Engine()
    {
        if (!Log::init()) {
            throw std::runtime_error("Failed to initialize log");
        }
        ORION_CORE_LOG_INFO("Orion Engine {} {} {}", ORION_OS_NAME, ORION_ARCH_NAME, ORION_COMPILER_NAME);
        if (!platform_init()) {
            throw std::runtime_error("Failed to initialize platform");
        }
        if (window_ = platform_window_create("Orion", 800, 600); !window_) {
            throw std::runtime_error("Failed to create main window");
        }
        if (!Renderer::init()) {
            throw std::runtime_error("Failed to initialize renderer");
        }
    }

    Engine::~Engine()
    {
        Renderer::shutdown();
        platform_window_destroy(window_);
        platform_shutdown();
    }

    void Engine::run(std::unique_ptr<Application> app)
    {
        const double dt = 1 / 60.0;

        double current_time = platform_time_get_time();
        double accumulator = 0.0;

        while (!platform_window_should_close(window_)) {
            double new_time = platform_time_get_time();
            double frame_time = new_time - current_time;
            current_time = new_time;

            if (frame_time > 0.25) {
                frame_time = 0.25;
            }

            accumulator += frame_time;

            platform_window_poll_events(window_);
            while (accumulator > dt) {
                app->update(static_cast<float>(dt));
                accumulator -= dt;
            }

            const double alpha = accumulator / dt;
            app->render(static_cast<float>(alpha));
        }
    }
} // namespace orion
