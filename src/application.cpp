#include "orion/application.hpp"

#include "orion/compiler.hpp"
#include "orion/config.h"
#include "orion/platform.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace orion
{
    Application::Application()
    {
        spdlog::set_level(static_cast<spdlog::level::level_enum>(SPDLOG_ACTIVE_LEVEL));
        SPDLOG_INFO("Orion Game Engine");
        SPDLOG_INFO("version: {}", ORION_VERSION);
        SPDLOG_INFO("platform: {}", ORION_PLATFORM_NAME);
        SPDLOG_INFO("build: {}", ORION_BUILD_TYPE);
        SPDLOG_INFO("compiler: {}", ORION_COMPILER_NAME);
    }

    void Application::update()
    {
        on_update();
    }

    void Application::render()
    {
        on_render();
    }

    void Application::orion_exit()
    {
        should_exit_ = true;
    }

    void Application::orion_abort(const std::string& what)
    {
        throw std::runtime_error(what);
    }
} // namespace orion
