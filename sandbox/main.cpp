#include <orion-engine/orion-engine.h>

#include <orion-core/window.h>

#include <orion-renderer/colors.h>
#include <orion-renderer/renderer.h>

#include <orion-math/vector/vector3.h>

#include <imgui.h>

class SandboxApp final : public orion::Application
{
public:
    SandboxApp()
        : window_({.name = "Sandbox App", .size = orion::default_window_size})
        , renderer_({.device_select_fn = orion::device_select_discrete, .render_size = {800, 600}})
        , render_window_(renderer_.create_render_window(window_))
        , quad_renderer_(renderer_.create_quad_renderer())
        , imgui_(renderer_.imgui_init(window_))
    {
        // Close app on callback
        window_.on_close().subscribe(ORION_EXIT_APP_FN);
    }

private:
    void on_user_update([[maybe_unused]] orion::FrameTime delta_time) override
    {
        window_.poll_events();
    }

    void render_scene()
    {
        renderer_.draw_test_triangle();

        quad_renderer_.begin();
        quad_renderer_.add({.position = {-0.75f, 0.f, 0.f}, .color = orion::colors::cyan});
        quad_renderer_.add({.position = {0.75f, 0.f, 0.f}, .color = orion::colors::green});
        renderer_.render(quad_renderer_);
    }

    void render_gui()
    {
        renderer_.imgui_new_frame();
        ImGui::ShowDemoWindow();
        renderer_.imgui_render();
    }

    void on_user_render() override
    {
        if (window_.is_minimized()) {
            return;
        }

        renderer_.begin();

        render_scene();

        render_gui();

        renderer_.end();

        renderer_.present(render_window_);
    }

    orion::Window window_;
    orion::Renderer renderer_;
    orion::RenderWindow render_window_;
    orion::QuadRenderer quad_renderer_;
    orion::ImGuiContext imgui_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
