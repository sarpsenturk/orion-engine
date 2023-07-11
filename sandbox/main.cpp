#include <fmt/chrono.h>
#include <orion-core/window.h>
#include <orion-engine/orion-engine.h>
#include <orion-math/vector/vector3.h>
#include <orion-renderer/renderer.h>
#include <spdlog/spdlog.h>

class SandboxApp : public orion::Application
{
public:
    SandboxApp()
        : window_({.name = "Orion Sandbox", .position = window_position, .size = window_size})
        , renderer_({.window = &window_})
    {
    }

private:
    void on_user_update(orion::frame_time dt) override
    {
        window_.poll_events();
        SPDLOG_LOGGER_DEBUG(logger(), "DT: {}", dt);
    }

    void on_user_render() override
    {
        renderer_.begin_frame();

        renderer_.end_frame();
        renderer_.present();
    }

    [[nodiscard]] bool user_should_exit() const noexcept override
    {
        return window_.should_close();
    }

    static constexpr auto window_position = orion::WindowPosition{400, 200};
    static constexpr auto window_size = orion::WindowSize{800, 600};

    orion::Window window_;
    orion::Renderer renderer_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
