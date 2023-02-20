#include "orion-engine/application.h"

#include "orion-core/config.h"

#include <spdlog/sinks/stdout_color_sinks.h> // spdlog::stdout_color_*
#include <spdlog/spdlog.h>                   // spdlog::set_pattern, spdlog::set_level, SPDLOG_*

namespace orion
{
    Application::Application()
    {
        // Set log level and format globally
        spdlog::set_pattern("[%n] [%^%l%$] %v");
        spdlog::set_level(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));

        // Create a new default logger with our name and set it as default
        auto logger = spdlog::stdout_color_st("orion-application");
        spdlog::set_default_logger(logger);

        SPDLOG_INFO("<Orion Engine> version: {}, platform: {}, debug_build: {}", current_version, to_string(current_platform), debug_build);
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
