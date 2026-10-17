#include "orion.hpp"

#include "orion/core/input.hpp"
#include "orion/core/logger.hpp"
#include "orion/core/platform.hpp"
#include "orion/core/window.hpp"
#include "orion/renderer/imgui_renderer.hpp"
#include "orion/renderer/renderer.hpp"

#include <chrono>

namespace orion
{
    void Application::update(float dt)
    {
        on_update(dt);
    }

    void Application::render(RenderFrame& frame)
    {
        on_render(frame);
    }

    Engine::Engine()
        : m_logger(std::make_unique<Logger>())
        , m_platform(std::make_unique<Platform>())
        , m_window(std::make_unique<Window>(WindowDesc{"Orion Engine", 800, 600}))
        , m_input(std::make_unique<Input>(m_window.get()))
        , m_renderer(std::make_unique<Renderer>(RendererDesc{m_window.get()}))
        , m_imgui_renderer(std::make_unique<ImGuiRenderer>(ImGuiRendererDesc{
              .window        = m_window.get(),
              .device        = m_renderer->device(),
              .image_count   = Renderer::swapchain_image_count,
              .target_format = Renderer::swapchain_format,
          }))
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
        using Clock     = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<Clock>;
        using Duration  = std::chrono::duration<float>;

        auto last_time = Clock::now();
        while (!m_window->should_close()) {
            m_platform->poll_events();

            const auto current_time = Clock::now();
            const auto delta_time   = Duration(current_time - last_time);
            last_time               = current_time;
            app->update(delta_time.count());

            if (auto frame = m_renderer->begin_frame()) {
                m_imgui_renderer->begin_frame();
                app->render(*frame);
                m_imgui_renderer->end_frame();
                m_renderer->end_frame();
                m_imgui_renderer->render_platform_windows();
            } else {
                const auto width  = m_window->width();
                const auto height = m_window->height();
                if (width > 0 && height > 0) {
                    m_renderer->resize_swapchain(static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height));
                }
            }
        }
    }
} // namespace orion
