#include "orion.hpp"

namespace orion
{
    void Application::update(float dt)
    {
        on_update(dt);
    }

    void Application::render()
    {
        on_render();
    }

    void Engine::run(std::unique_ptr<Application> app)
    {
        while (true) {
            app->update(0.0f);
            app->render();
        }
    }
} // namespace orion
