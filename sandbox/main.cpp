#include <orion-engine/orion-engine.h>

#include <orion-core/window.h>

#include <orion-renderer/camera.h>
#include <orion-renderer/renderer.h>

#include <spdlog/spdlog.h>

#include <array>

class SandboxApp final : public orion::Application
{
public:
    static constexpr auto vertices = std::array{
        orion::Vertex{{-.5f, 0.5f, 1.f}},
        orion::Vertex{{0.5f, 0.5f, 1.f}},
        orion::Vertex{{0.5f, -.5f, 1.f}},
        orion::Vertex{{-.5f, -.5f, 1.f}},
    };
    static constexpr auto indices = std::array{0u, 1u, 2u, 2u, 3u, 0u};

    SandboxApp()
        : window_({.name = "Sandbox App", .position = orion::defaults::window_position, .size = orion::defaults::window_size})
        , renderer_({.render_size = window_.size()})
        , quad_mesh_(renderer_.mesh_builder().create_mesh(vertices, indices))
        , effect_(renderer_.effect_compiler().compile_file(orion::input_file("assets/effects/default.ofx"), {.shader_base_path = "assets/shaders/spirv"}))
        , blue_mat_(renderer_.material_builder().create(&effect_, {.color = orion::colors::blue}))
        , green_mat_(renderer_.material_builder().create(&effect_, {.color = orion::colors::lime}))
        , current_mat_(&blue_mat_)
        , render_window_(renderer_.create_render_window(window_))
        , camera_controller_(&camera_, orion::degrees(45.f), window_.aspect_ratio(), {0, 0, 5})
    {
    }

private:
    void change_material()
    {
        if (current_mat_ == &blue_mat_) {
            current_mat_ = &green_mat_;
        } else {
            current_mat_ = &blue_mat_;
        }
    }

    void on_user_update([[maybe_unused]] orion::FrameTime delta_time) override
    {
        orion::WindowEvent event;
        while ((event = window_.poll_event())) {
            if (event.get_if<orion::WindowClose>()) {
                exit_application();
                continue;
            }
            if (const auto* keydown = event.get_if<orion::KeyDown>()) {
                if (keydown->key == orion::KeyCode::Enter) {
                    change_material();
                }
                continue;
            }
        }

        if (window_.keyboard().is_down(orion::KeyCode::KeyW)) {
            camera_controller_.translate(camera_controller_.forward() * camera_speed_ * delta_time);
        }
        if (window_.keyboard().is_down(orion::KeyCode::KeyS)) {
            camera_controller_.translate(-camera_controller_.forward() * camera_speed_ * delta_time);
        }
        if (window_.keyboard().is_down(orion::KeyCode::KeyD)) {
            camera_controller_.translate(camera_controller_.right() * camera_speed_ * delta_time);
        }
        if (window_.keyboard().is_down(orion::KeyCode::KeyA)) {
            camera_controller_.translate(-camera_controller_.right() * camera_speed_ * delta_time);
        }
        if (window_.keyboard().is_down(orion::KeyCode::Control)) {
            camera_controller_.translate(orion::vec3(0.f, -1.f, 0.f) * camera_speed_ * delta_time);
        }
        if (window_.keyboard().is_down(orion::KeyCode::Shift)) {
            camera_controller_.translate(orion::vec3(0.f, 1.f, 0.f) * camera_speed_ * delta_time);
        }
        if (window_.keyboard().is_down(orion::KeyCode::RightArrow)) {
            camera_controller_.set_yaw(camera_controller_.yaw() + orion::degrees(45.f) * delta_time.count());
        }
        if (window_.keyboard().is_down(orion::KeyCode::LeftArrow)) {
            camera_controller_.set_yaw(camera_controller_.yaw() - orion::degrees(45.f) * delta_time.count());
        }
        if (window_.keyboard().is_down(orion::KeyCode::UpArrow)) {
            camera_controller_.set_pitch(camera_controller_.pitch() + orion::degrees(45.f) * delta_time.count());
        }
        if (window_.keyboard().is_down(orion::KeyCode::DownArrow)) {
            camera_controller_.set_pitch(camera_controller_.pitch() - orion::degrees(45.f) * delta_time.count());
        }
    }

    void on_user_render() override
    {
        if (window_.is_minimized()) {
            return;
        }

        renderer_.draw({&quad_mesh_, current_mat_, &transform});
        renderer_.render(camera_);
        renderer_.present(render_window_);
    }

    orion::Window window_;
    orion::Renderer renderer_;
    orion::Mesh quad_mesh_;
    orion::Effect effect_;
    orion::Material blue_mat_;
    orion::Material green_mat_;
    orion::Material* current_mat_;
    orion::Matrix4_f transform = orion::Matrix4_f::identity();
    orion::RenderWindow render_window_;
    orion::Camera camera_;
    orion::CameraController camera_controller_;
    float camera_speed_ = 2.f;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
