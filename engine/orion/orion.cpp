#include "orion.hpp"

#include "orion/core/logger.hpp"

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

    Engine::Engine()
        : m_logger(std::make_unique<Logger>())
    {
        ORION_LOG_INFO(LogChannel::Core, "Orion Engine");
    }

    // Needs to be explicitly defaulted here where members are complete types.
    Engine::~Engine() = default;

    void Engine::run(std::unique_ptr<Application> app)
    {
        while (!app->should_exit()) {
            app->update();
            app->render();
        }
    }
} // namespace orion
