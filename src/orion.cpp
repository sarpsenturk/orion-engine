#include "orion/orion.hpp"

#include "orion/log.hpp"

#include "orion/platform/platform.hpp"
#include "orion/platform/window.hpp"

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
    }

    Engine::~Engine()
    {
        platform_window_destroy(window_);
        platform_shutdown();
    }

    void Engine::run(std::unique_ptr<Application> app)
    {
        while (!platform_window_should_close(window_)) {
            platform_window_poll_events(window_);
            app->update();
            app->render();
        }
    }
} // namespace orion
