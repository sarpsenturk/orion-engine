#include "orion-engine/application.h"

namespace orion
{
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
} // namespace orion
