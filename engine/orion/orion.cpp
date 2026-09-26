#include "orion.hpp"

#include "orion/core/logger.hpp"
#include "orion/core/platform.hpp"
#include "orion/core/window.hpp"
#include "orion/renderer/renderer.hpp"

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

    Engine::Engine()
        : m_logger(std::make_unique<Logger>())
        , m_platform(std::make_unique<Platform>())
        , m_window(std::make_unique<Window>(WindowDesc{"Orion Engine", 800, 600}))
        , m_renderer(std::make_unique<Renderer>(RendererDesc{m_window.get()}))
    {
        ORION_LOG_INFO(LogChannel::Core, "Orion Engine");
        ORION_LOG_INFO(LogChannel::Core, "Platform: {} {}", m_platform->name(), m_platform->version());
        ORION_LOG_INFO(LogChannel::Core, "GPU Backend: {} {}", gpu::backend_name(), gpu::backend_version());
        ORION_LOG_INFO(LogChannel::Core, "GPU Device: {}", gpu::device_name(m_renderer->device()));
    }

    // Needs to be explicitly defaulted here where members are complete types.
    Engine::~Engine() = default;

    void Engine::run(std::unique_ptr<Application> app)
    {
        while (!m_window->should_close()) {
            m_platform->poll_events();
            app->update(0.0f);
            app->render();
        }
    }
} // namespace orion
