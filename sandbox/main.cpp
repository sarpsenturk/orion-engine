#include <orion-engine/orion-engine.h>

#include <orion-core/window.h>

#include <orion-renderer/renderer.h>

#include <orion-math/vector/vector3.h>

#include <imgui.h>

class SandboxApp final : public orion::Application
{
public:
    SandboxApp()
        : window_({.name = "Sandbox App"})
        , renderer_({.render_size = window_.size()})
    {
        // Close app on callback
        window_.on_close().subscribe(ORION_EXIT_APP_FN);
    }

private:
    void on_user_update([[maybe_unused]] orion::FrameTime delta_time) override
    {
        window_.poll_events();
    }

    void on_user_render() override
    {
        if (window_.is_minimized()) {
            return;
        }
    }

    orion::Window window_;
    orion::Renderer renderer_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
