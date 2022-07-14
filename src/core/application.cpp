#include "orion/core/application.h"

namespace orion
{
    void Application::on_create(const argparse::ArgParse& args)
    {
        on_user_create(args);
    }

    void Application::on_shutdown() { on_user_shutdown(); }

    void Application::on_update() { on_user_update(); }

    bool Application::should_close() const noexcept
    {
        return user_should_close();
    }
} // namespace orion
