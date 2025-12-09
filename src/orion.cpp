#include "orion/orion.hpp"

#include "orion/log.hpp"

#include <stdexcept>

namespace orion
{
    Engine::Engine()
    {
        if (!Log::init()) {
            throw std::runtime_error("Failed to initialize log");
        }
        ORION_CORE_LOG_INFO("Hello world");
    }

    void Engine::run(std::unique_ptr<Application> app)
    {
        while (!app->should_close()) {
            app->update();
            app->render();
        }
    }
} // namespace orion
