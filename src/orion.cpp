#include "orion/orion.h"

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

// Temporary entry point definition
// This will change as different entry points are needed
// for different platforms
int main(int argc, const char* argv[])
{
    try {
        auto app = create_orion_app(std::span{argv, static_cast<std::size_t>(argc)});
        while (!app->should_exit()) {
            app->update();
            app->render();
        }
        return 0;
    } catch (const std::exception& err) {
        std::fputs(err.what(), stderr);
        return 1;
    } catch (...) {
        std::fputs("unknown exception", stderr);
        return 1;
    }
}
