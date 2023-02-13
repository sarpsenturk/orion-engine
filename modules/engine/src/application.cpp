#include "orion-engine/application.h"

#include "orion-core/config.h"

#include <spdlog/spdlog.h> // spdlog::set_pattern, spdlog::set_level, SPDLOG_*

namespace orion
{
    Application::Application()
    {
        spdlog::set_pattern("[%^%l%$] %v");
        spdlog::set_level(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
        SPDLOG_INFO("<Orion Engine> version: {}, platform: {}, debug_build: {}", current_version, current_platform, debug_build);
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
