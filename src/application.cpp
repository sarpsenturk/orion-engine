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
} // namespace orion
