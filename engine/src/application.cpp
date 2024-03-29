#include "orion-engine/application.h"

#ifndef ORION_APPLICATION_LOG_LEVEL
    #define ORION_APPLICATION_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    Application::Application()
        : logger_(orion::create_logger("orion-application", ORION_APPLICATION_LOG_LEVEL))
    {
        SPDLOG_LOGGER_INFO(logger(), "<Orion Engine> version: {}, platform: {}, build_type: {}",
                           ORION_VERSION, ORION_PLATFORM, ORION_BUILD_TYPE);
    }

    void Application::on_update(FrameTime dt)
    {
        on_user_update(dt);
    }

    void Application::on_render()
    {
        on_user_render();
    }

    bool Application::should_exit() const noexcept
    {
        return should_exit_;
    }

    void Application::run()
    {
        Clock::time_point last_frame;
        while (!should_exit()) {
            const auto now = Clock::now();
            const auto dt = now - last_frame;
            last_frame = now;
            on_update(dt);
            on_render();
        }
    }

    void Application::exit_application()
    {
        should_exit_ = true;
    }
} // namespace orion
