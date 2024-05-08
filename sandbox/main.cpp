#include <orion-engine/orion-engine.h>

#include <orion-core/window.h>

#include <orion-renderer/colors.h>
#include <orion-renderer/renderer.h>
#include <orion-renderer/texture.h>

#include <orion-math/vector/vector3.h>

#include <imgui.h>

class SandboxApp final : public orion::Application
{
public:
    SandboxApp()
        : window_({.name = "Sandbox App"})
        , renderer_({.device_select_fn = orion::device_select_discrete, .render_size = {800, 600}})
        , render_window_(renderer_.create_render_window(window_, false))
        , quad_renderer_(renderer_.create_quad_renderer())
        , imgui_(renderer_.imgui_init(window_))
        , quad_texture_(renderer_.texture_loader().load_from_file(ORION_TEXTURE_DIR "/uv-checker.png").value())
        , quad_texture_index_(renderer_.texture_array().add(quad_texture_))
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
        quad_renderer_.begin();
        quad_renderer_.add({
            .position = {0.f, 0.f, 0.f},
            .rotation = orion::to_radians(rotation_),
            .scale = orion::vec2(scale_, scale_),
            .color = quad_color,
            .texture_index = use_texture_ ? quad_texture_index_ : orion::textures::white,
        });
        renderer_.render(quad_renderer_);
    }

    void render_gui()
    {
        ImGui::ShowDemoWindow();

        ImGui::Begin("Quads");
        ImGui::DragFloat("Rotation", rotation_.value_ptr());
        ImGui::DragFloat("Scale", &scale_);
        ImGui::Checkbox("Use texture?", &use_texture_);
        ImGui::ColorEdit4("Color", quad_color.data());
        ImGui::End();
    }

    void on_user_render() override
    {
        if (window_.is_minimized()) {
            return;
        }

        renderer_.begin();

        render_scene();

        renderer_.imgui_new_frame();
        render_gui();
        renderer_.imgui_render();

        renderer_.end();

        renderer_.present(render_window_);
    }

    orion::Window window_;
    orion::Renderer renderer_;
    orion::RenderWindow render_window_;
    orion::QuadRenderer quad_renderer_;
    orion::ImGuiContext imgui_;
    orion::Texture quad_texture_;
    orion::texture_index_t quad_texture_index_;

    orion::Degree_f rotation_ = orion::degrees(0.f);
    float scale_ = 1.f;
    bool use_texture_ = false;
    orion::Color quad_color = orion::colors::white;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
