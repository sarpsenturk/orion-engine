#include "orion/orion.hpp"

#include <exception>

namespace orion
{
    Engine::Engine()
        : logger_(std::make_unique<Logger>())
    {
        ORION_CORE_LOG_TRACE("{}", "trace");
        ORION_CORE_LOG_DEBUG("{}", "debug");
        ORION_CORE_LOG_INFO("{}", "info");
        ORION_CORE_LOG_WARN("{}", "warn");
        ORION_CORE_LOG_ERROR("{}", "error");
    }

    void Engine::run(std::unique_ptr<Application> app)
    {
        try {
            while (!app->should_exit()) {
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
