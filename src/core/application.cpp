#include "orion/core/application.h"

namespace orion
{
    void Application::on_create(const argparse::ArgParse& args)
    {
        on_user_create(args);
    }

    void Application::on_shutdown() { on_user_shutdown(); }

    void Application::run()
    {
        // TODO: Run main application loop
    }
} // namespace orion
