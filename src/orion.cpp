#include "orion/orion.hpp"

#include "orion/config.h"
#include "orion/platform.hpp"

#include <fmt/std.h>

#include <exception>

namespace orion
{
    Engine::Engine()
        : logger_(std::make_unique<Logger>())
        , window_(std::make_unique<Window>(WindowDesc{.title = "Orion", .width = 800, .height = 600}))
    {
        ORION_CORE_LOG_INFO("Orion Engine v{} {}-{}-{}-{}",
                            ORION_VERSION, ORION_BUILD_TYPE, ORION_ARCH_NAME, ORION_COMPILER_NAME, ORION_PLATFORM_NAME);

        window_->set_event_callback([this](const WindowEvent& event) { on_event(event); });
    }

    void Engine::run(std::unique_ptr<Application> app)
    {
        try {
            while (!app->should_exit() && !window_->should_close()) {
                window_->poll_events();
                app->update();
                app->render();
            }
        } catch (const std::exception& ex) {
            ORION_CORE_LOG_ERROR("Fatal exception: {}", ex.what());
        } catch (...) {
            ORION_CORE_LOG_ERROR("Fatal exception: unknown");
        }
    }

    void Engine::on_event(const WindowEvent& event)
    {
        ORION_CORE_LOG_TRACE("[WindowEvent] {}", event.payload());
    }
} // namespace orion
