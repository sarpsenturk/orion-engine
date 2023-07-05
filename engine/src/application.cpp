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

    void Application::on_update()
    {
        on_user_update();
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
        while (!should_exit()) {
            on_update();
            on_render();
        }
    }
} // namespace orion
