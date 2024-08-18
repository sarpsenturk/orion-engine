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
        , render_device_(render_backend_->create_device(0))
        , command_queue_(render_device_->create_command_queue())
    {
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
    std::unique_ptr<RenderDevice> render_device_;
    std::unique_ptr<CommandQueue> command_queue_;
};

std::unique_ptr<Application> orion_main(std::span<const char* const> args)
{
    return std::make_unique<SandboxApp>();
}
