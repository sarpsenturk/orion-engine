#include "orion.hpp"

#include "orion/core/logger.hpp"
#include "orion/core/platform.hpp"
#include "orion/renderer/renderer.hpp"

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

    void Application::event(const Event& event)
    {
        on_event(event);
    }

    Engine::Engine()
        : m_logger(std::make_unique<Logger>())
        , m_platform(std::make_unique<Platform>())
        , m_renderer(std::make_unique<Renderer>())
    {
        ORION_LOG_INFO(LogChannel::Core, "Orion Engine");
        ORION_LOG_INFO(LogChannel::Core, "Platform: {} {}", m_platform->name(), m_platform->version());
        ORION_LOG_INFO(LogChannel::Core, "GPU Backend: {} {}", gpu::backend_name(), gpu::backend_version());
    }

    // Needs to be explicitly defaulted here where members are complete types.
    Engine::~Engine() = default;

    void Engine::run(std::unique_ptr<Application> app)
    {
        while (!app->should_exit()) {
            m_platform->pump_events();
            Event event;
            while (m_platform->poll_event(event)) {
                app->event(event);
            }

            app->update();
            app->render();
        }
    }
} // namespace orion
