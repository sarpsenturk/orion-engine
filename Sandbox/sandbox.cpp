#include "orion/events/window_event.h"
#include "orion/orion.h"
#include "orion/window/window.h"

#include <spdlog/spdlog.h>

class Sandbox
    : public orion::Application
    , orion::WindowEventHandler
    , orion::MouseEventHandler
{
public:
    static constexpr orion::Vector2_i kWindowSize{720, 480};

    Sandbox()
        : window_({"Sandbox", kWindowSize})
    {
        window_.window_events().attach<orion::WindowCreateEvent>(this);
        window_.window_events().attach<orion::WindowCloseEvent>(this);
        window_.window_events().attach<orion::WindowMoveEvent>(this);
        window_.window_events().attach<orion::WindowResizeEvent>(this);
        window_.window_events().attach<orion::WindowFocusEvent>(this);
        window_.mouse_events().attach<orion::MouseMoveEvent>(this);
    }

private:
    void on_user_create(const argparse::ArgParse& args) override
    {
        spdlog::info("on_create()");
        window_.initialize();
    }

    void on_user_shutdown() override
    {
        spdlog::info("on_shutdown()");
        window_.destroy();
    }

    void on_user_update() override { window_.update(); }

    bool user_should_close() const noexcept override
    {
        return window_.should_close();
    }

    void process(const orion::WindowCreateEvent& event) override
    {
        spdlog::info("Window Created: {}", event.window_name);
    }

    void process(const orion::WindowCloseEvent& event) override
    {
        spdlog::info("Window Closed: {}", event.window_name);
    }

    void process(const orion::WindowMoveEvent& event) override
    {
        spdlog::info("Window Moved: {}, x: {}, y: {}", event.window_name,
                     event.position.x(), event.position.y());
    }

    void process(const orion::WindowResizeEvent& event) override
    {
        spdlog::info("Window Resized: {}, width: {}, height: {}",
                     event.window_name, event.size.x(), event.size.y());
    }

    void process(const orion::WindowFocusEvent& event) override
    {
        spdlog::info("Window Focus Changed: {}, is_focused: {}",
                     event.window_name, event.is_focused);
    }

    void process(const orion::MouseMoveEvent& event) override
    {
        spdlog::info("Mouse Moved: x: {}, y: {}", event.position.x(),
                     event.position.y());
    }

    orion::Window window_;
};

auto orion::create_application() -> std::unique_ptr<Application>
{
    return std::make_unique<Sandbox>();
}
