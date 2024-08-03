#include "orion/application.h"

#include <cstdio>
#include <exception>

namespace orion
{
    Application::Application()
        : console_out_(make_stdout_logger(LogLevel::Trace))
        , console_err_(make_stderr_logger(LogLevel::Trace))
    {
    }

    void Application::update()
    {
        on_update();
    }

    void Application::render()
    {
        on_render();
    }

    void Application::exit()
    {
        should_exit_ = true;
    }
} // namespace orion
