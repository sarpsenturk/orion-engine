#include "orion/application.hpp"

namespace orion
{
    void Application::update()
    {
        on_update();
    }

    void Application::render()
    {
        on_render();
    }

    bool Application::should_close() const
    {
        return on_should_close();
    }
} // namespace orion
