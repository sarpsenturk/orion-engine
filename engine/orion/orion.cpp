#include "orion.hpp"

#include "orion/core/logger.hpp"
#include "orion/core/platform.hpp"

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
        , m_platform(std::make_unique<Platform>())
    {
        ORION_LOG_INFO(LogChannel::Core, "Orion Engine");
        ORION_LOG_INFO(LogChannel::Core, "Platform: {} {}", m_platform->name(), m_platform->version());
    }

    // Needs to be explicitly defaulted here where members are complete types.
    Engine::~Engine() = default;

    void Engine::run(std::unique_ptr<Application> app)
    {
        while (!app->should_exit()) {
            m_platform->poll_events();
            app->update();
            app->render();
        }
    }
} // namespace orion
