#include <orion-engine/orion-engine.h>

#include <orion-core/window.h>

#include <orion-renderer/renderer.h>

#include <orion-math/vector/vector3.h>

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
    {
    }

private:
    void on_user_update([[maybe_unused]] orion::FrameTime delta_time) override
    {
        static int value = 0;
        orion::WindowEvent event;
        while ((event = window_.poll_event())) {
            if (event.get_if<orion::WindowClose>()) {
                exit_application();
                continue;
            }
            if (event.get_if<orion::WindowMove>()) {
                SPDLOG_LOGGER_INFO(logger(), "{}", value++);
                continue;
            }
        }
    }

    void on_user_render() override
    {
        renderer_.draw({&quad_mesh_, current_mat_});
        renderer_.render();
        renderer_.present(render_window_);
    }

    orion::Window window_;
    orion::Renderer renderer_;
    orion::Mesh quad_mesh_;
    orion::Effect effect_;
    orion::Material blue_mat_;
    orion::Material green_mat_;
    orion::Material* current_mat_;
    orion::RenderWindow render_window_;
};

ORION_MAIN(args)
{
    SandboxApp sandbox;
    sandbox.run();
    return 0;
}
