#include "orion/orion.hpp"

#include "orion/config.h"
#include "orion/platform.hpp"

#include <exception>

namespace orion
{
    Engine::Engine()
        : logger_(std::make_unique<Logger>())
        , window_(std::make_unique<Window>(WindowDesc{.title = "Orion", .width = 800, .height = 600}))
    {
        ORION_CORE_LOG_INFO("Orion Engine v{} {}-{}-{}-{}",
                            ORION_VERSION, ORION_BUILD_TYPE, ORION_ARCH_NAME, ORION_COMPILER_NAME, ORION_PLATFORM_NAME);
    }

    void Engine::run(std::unique_ptr<Application> app)
    {
        try {
            while (!app->should_exit()) {
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
} // namespace orion
