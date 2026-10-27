#include "orion.hpp"

#include "orion/core/logger.hpp"
#include "orion/core/platform.hpp"
#include "orion/renderer/renderer.hpp"

#include <chrono>

namespace orion
{
    void Application::update(float delta_time)
    {
        on_update(delta_time);
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
        , m_renderer(std::make_unique<Renderer>(RendererDesc{.width = 1'280, .height = 720}))
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
        using Clock     = std::chrono::steady_clock;
        using Duration  = std::chrono::duration<float, std::chrono::seconds::period>;
        using TimePoint = std::chrono::time_point<Clock, Duration>;

        auto last_time = Clock::now();
        while (!app->should_exit()) {
            const auto now        = Clock::now();
            const auto delta_time = Duration{now - last_time};
            last_time             = now;

            m_platform->pump_events();
            Event event;
            while (m_platform->poll_event(event)) {
                on_event(event);   // Handle event at engine level first
                app->event(event); // Then pass event onto the application
            }

            app->update(delta_time.count());
            app->render();
        }
    }

    void Engine::on_event(const Event& event)
    {
        m_renderer->on_event(event);
    }
} // namespace orion
