#include <orion/application.h>
#include <orion/renderapi/render_backend.h>
#include <orion/window.h>

#include <spdlog/spdlog.h>

using namespace orion;

class SandboxApp final : public Application
{
public:
    SandboxApp()
        : window_({.title = "Sandbox App", .width = 800, .height = 600})
        , render_backend_(RenderBackend::create())
    {
        SPDLOG_INFO("Render backend: {}", render_backend_->name());
        for (const auto adapters = render_backend_->get_adapters(); const auto& adapter : adapters) {
            SPDLOG_INFO("Adapter: {}", adapter.name);
        }
    }

private:
    void on_update() override
    {
        WindowEvent event;
        while ((event = window_.poll_event())) {
            SPDLOG_TRACE("{}", event);
            if (event.is<OnWindowClose>()) {
                exit_application();
            }
        }
    }

    void on_render() override
    {
    }

    Window window_;
    std::unique_ptr<RenderBackend> render_backend_;
};

std::unique_ptr<Application> orion_main(std::span<const char* const> args)
{
    return std::make_unique<SandboxApp>();
}
