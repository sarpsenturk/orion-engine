#include <orion/application.h>
#include <orion/window.h>

using namespace orion;

class SandboxApp final : public Application
{
public:
    SandboxApp()
    {
        window_ = Window::create({.title = "Sandbox App", .width = 800, .height = 600})
                      .or_else([this](const WindowCreateError& error) { abort_application(error.msg); })
                      .value();
    }

private:
    void on_update() override
    {
        WindowEvent event;
        while ((event = window_->poll_event())) {
            cout()->trace("{}", event);
            if (event.is<OnWindowClose>()) {
                exit_application();
            }
        }
    }

    void on_render() override
    {
    }

    std::unique_ptr<Window> window_;
};

std::unique_ptr<Application> orion_main(std::span<const char* const> args)
{
    return std::make_unique<SandboxApp>();
}
