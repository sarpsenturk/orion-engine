#include <orion/application.h>
#include <orion/window.h>

#include <orion/renderapi/render_backend.h>
#include <orion/renderapi/shader.h>

#include <spdlog/spdlog.h>

using namespace orion;

constexpr const char* vertex_shader = R"hlsl(
float4 main(float3 position : POSITION) : SV_Position
{
    return float4(position, 1.0);
})hlsl";

constexpr const char* pixel_shader = R"hlsl(
float4 main() : SV_Target
{
    return float4(1.0, 1.0, 1.0, 1.0);
}
)hlsl";

class SandboxApp final : public Application
{
public:
    SandboxApp()
        : window_({.title = "Sandbox App", .width = 800, .height = 600})
        , render_backend_(RenderBackend::create_builtin_d3d12())
        , render_device_(render_backend_->create_device(0))
        , command_queue_(render_device_->create_command_queue())
        , swapchain_(render_device_->create_swapchain({.window = &window_, .queue = command_queue_.get(), .width = 800, .height = 600, .image_count = 2, .image_format = Format::B8G8R8A8_Unorm}))
    {
        auto compiler = render_device_->create_shader_compiler();
        const auto vs = compiler->compile({.source = vertex_shader, .type = ShaderType::Vertex});
        SPDLOG_DEBUG("Created vertex shader binary with size {}", vs.size());
        const auto ps = compiler->compile({.source = pixel_shader, .type = ShaderType::Pixel});
        SPDLOG_DEBUG("Created pixel shader binary with size {}", ps.size());

        pipeline_layout_ = render_device_->create_pipeline_layout({});

        graphics_pipeline_ = render_device_->create_graphics_pipeline({
            .pipeline_layout = pipeline_layout_,
            .vertex_shader = vs,
            .pixel_shader = ps,
            .vertex_attributes = {{VertexAttribute{.name = "POSITION", .format = Format::R32G32B32_Float}}},
            .primitive_topology = PrimitiveTopology::Triangle,
            .rasterizer = {.fill_mode = FillMode::Solid, .cull_mode = CullMode::Back, .front_face = FrontFace::ClockWise},
            .blend = {
                .render_targets = {{
                    RenderTargetBlendDesc{
                        .blend_enable = true,
                        .src_blend = Blend::One,
                        .dst_blend = Blend::Zero,
                        .blend_op = BlendOp::Add,
                        .src_blend_alpha = Blend::One,
                        .dst_blend_alpha = Blend::Zero,
                        .blend_op_alpha = BlendOp::Add,
                        .color_write_mask = ColorWriteFlags::All,
                    },
                }},
                .blend_constants = {1.f, 1.f, 1.f, 1.f},
            },
            .render_target_formats = {{Format::B8G8R8A8_Unorm}},
        });
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
    std::unique_ptr<Swapchain> swapchain_;
    PipelineLayoutHandle pipeline_layout_;
    PipelineHandle graphics_pipeline_;
};

std::unique_ptr<Application> orion_main(std::span<const char* const> args)
{
    return std::make_unique<SandboxApp>();
}
