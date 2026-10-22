#include "orion.hpp"

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

    void Engine::run(std::unique_ptr<Application> app)
    {
        while (!app->should_exit()) {
            app->update();
            app->render();
        }
    }
} // namespace orion
