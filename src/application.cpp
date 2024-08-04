#include "orion/application.h"

#include "orion/compiler.h"
#include "orion/config.h"
#include "orion/platform.h"

#include <stdexcept>

namespace orion
{
    Application::Application()
        : console_out_(make_stdout_logger(LogLevel::Trace))
        , console_err_(make_stderr_logger(LogLevel::Trace))
    {
        console_out_->info("Orion Game Engine");
        console_out_->info("version: {}", ORION_VERSION);
        console_out_->info("platform: {}", ORION_PLATFORM_NAME);
        console_out_->info("build: {}", ORION_BUILD_TYPE);
        console_out_->info("compiler: {}", ORION_COMPILER_NAME);
    }

    void Application::update()
    {
        on_update();
    }

    void Application::render()
    {
        on_render();
    }

    void Application::exit_application()
    {
        should_exit_ = true;
    }

    void Application::abort_application(const std::string& what)
    {
        throw std::runtime_error(what);
    }
} // namespace orion
