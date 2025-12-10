#include "orion/application.hpp"

namespace orion
{
    void Application::update(float dt)
    {
        on_update(dt);
    }

    void Application::render(float alpha)
    {
        on_render(alpha);
    }
} // namespace orion
