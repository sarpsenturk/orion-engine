#include <orion-engine/orion-engine.h>

#include <orion-core/window.h>

#include <orion-renderer/camera.h>
#include <orion-renderer/renderer.h>

#include <orion-math/matrix/transformation.h>

#include <spdlog/spdlog.h>

#include <imgui.h>

#include <array>

class SandboxImGui final : public orion::ImGuiLayer
{
    void on_user_draw() override
    {
        ImGui::ShowDemoWindow();
    }
};

class SandboxApp final : public orion::Application
{
public:
    static constexpr auto vertices = std::array{
        orion::Vertex{{-.5f, 0.5f, 1.f}, {0.f, 0.f}},
        orion::Vertex{{0.5f, 0.5f, 1.f}, {1.f, 0.f}},
        orion::Vertex{{0.5f, -.5f, 1.f}, {1.f, 1.f}},
        orion::Vertex{{-.5f, -.5f, 1.f}, {0.f, 1.f}},
    };
    static constexpr auto indices = std::array{0u, 1u, 2u, 2u, 3u, 0u};

    static constexpr auto window_size = orion::defaults::window_size;

    SandboxApp()
        : renderer_({.render_size = window_size})
        , window_({.name = "Sandbox App", .position = orion::defaults::window_position, .size = window_size})
        , swapchain_(renderer_.create_swapchain(window_))
        , quad_mesh_(renderer_.create_mesh(vertices, indices).first)
        , blue_mat_(renderer_.create_material({.color = orion::colors::blue}).first)
        , green_mat_(renderer_.create_material({.color = orion::colors::lime}).first)
        , current_mat_(blue_mat_)
        , camera_controller_(&camera_, orion::degrees(45.f), window_.aspect_ratio(), {0, 0, 5})
        , imgui_(renderer_.create_imgui_layer<SandboxImGui>())
    {
    }

private:
    void change_material()
    {
        if (current_mat_ == blue_mat_) {
            current_mat_ = green_mat_;
        } else {
            current_mat_ = blue_mat_;
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
            imgui_->on_event(event);
            if (const auto* keydown = event.get_if<orion::KeyDown>()) {
                if (keydown->key == orion::KeyCode::Enter) {
                    change_material();
                } else if (keydown->key == orion::KeyCode::Escape) {
                    exit_application();
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

        transform = transform * orion::rotation_z(orion::degrees(-15.f) * delta_time.count());
    }

    void on_user_render() override
    {
        if (window_.is_minimized()) {
            return;
        }

        renderer_.draw({quad_mesh_, current_mat_, &transform});
        renderer_.render(camera_);
        renderer_.present_to(swapchain_.get());
        swapchain_->present(1);
    }

    orion::Renderer renderer_;
    orion::Window window_;
    orion::SwapchainPtr swapchain_;
    orion::mesh_id_t quad_mesh_;
    orion::material_id_t blue_mat_;
    orion::material_id_t green_mat_;
    orion::material_id_t current_mat_;
    orion::Matrix4_f transform = orion::Matrix4_f::identity();
    orion::Camera camera_;
    orion::CameraController camera_controller_;
    float camera_speed_ = 2.f;
    std::unique_ptr<orion::ImGuiLayer> imgui_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
