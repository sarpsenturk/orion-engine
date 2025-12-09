#include "orion/orion.hpp"

#include <cstdio>

namespace orion
{
    Engine::Engine()
    {
        std::puts("Hello world");
    }

    void Engine::run(std::unique_ptr<Application> app)
    {
        while (!app->should_close()) {
            app->update();
            app->render();
        }
    }
} // namespace orion
