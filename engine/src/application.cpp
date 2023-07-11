#include "orion-engine/application.h"

#include "orion-core/config.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace orion
{
    Application::Application()
        : logger_(spdlog::stdout_color_st("orion-application"))
    {
        logger_->set_pattern("[%n] [%^%l%$] %v");
        logger_->set_level(static_cast<spdlog::level::level_enum>(ORION_APPLICATION_LOG_LEVEL));

        SPDLOG_LOGGER_INFO(logger(), "<Orion Engine> version: {}, platform: {}, debug_build: {}",
                           current_version, current_platform, debug_build);
    }

    void Application::on_update(frame_time dt)
    {
        on_user_update(dt);
    }

    void Application::on_render()
    {
        on_user_render();
    }

    bool Application::should_exit() const noexcept
    {
        return user_should_exit();
    }

    void Application::run()
    {
        clock::time_point last_frame;
        while (!should_exit()) {
            const auto now = clock::now();
            const auto dt = now - last_frame;
            last_frame = now;
            on_update(dt);
            on_render();
        }
    }
} // namespace orion
