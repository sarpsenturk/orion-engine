#include "orion/core/application.h"

namespace orion
{
    void Application::on_create() { on_user_create(); }

    void Application::on_shutdown() { on_user_shutdown(); }

    void Application::run()
    {
        // TODO: Run main application loop
    }
} // namespace orion
