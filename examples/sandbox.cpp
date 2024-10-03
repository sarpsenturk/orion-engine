#include <orion/application.h>
#include <orion/window.h>

#include <orion/renderapi/render_backend.h>
#include <orion/renderapi/shader.h>

#include <orion/math/vector3.h>

#include <spdlog/spdlog.h>

#include <array>

using namespace orion;

constexpr auto vertex_shader = R"hlsl(
float4 main(float3 position : POSITION) : SV_Position
{
    return float4(position, 1.0);
})hlsl";

constexpr auto pixel_shader = R"hlsl(
float4 main() : SV_Target
{
    return float4(1.0, 1.0, 1.0, 1.0);
}
)hlsl";

struct Vertex {
    Vector3f position;
};

constexpr auto vertices = std::array{
    Vertex{.position = {0.f, .5f, 0.f}},
    Vertex{.position = {.5f, -.5f, 0.f}},
    Vertex{.position = {-.5f, -.5f, 0.f}},
};

class SandboxApp final : public Application
{
public:
    SandboxApp()
        : window_({.title = "Sandbox App", .width = 800, .height = 600})
        , render_backend_(RenderBackend::create_builtin_vulkan())
        , render_device_(render_backend_->create_device(0))
        , command_queue_(render_device_->create_command_queue())
        , swapchain_(render_device_->create_swapchain({.window = &window_, .queue = command_queue_.get(), .width = 800, .height = 600, .image_count = 2, .image_format = Format::B8G8R8A8_Unorm}))
        , command_allocator_(render_device_->create_command_allocator({}))
        , command_list_(render_device_->create_command_list({.command_allocator = command_allocator_.get()}))
        , fence_(render_device_->create_fence({.signaled = false}))
    {
        // Compile shaders
        auto compiler = render_device_->create_shader_compiler();
        const auto vs = compiler->compile({.source = vertex_shader, .type = ShaderType::Vertex});
        SPDLOG_DEBUG("Created vertex shader binary with size {}", vs.size());
        const auto ps = compiler->compile({.source = pixel_shader, .type = ShaderType::Pixel});
        SPDLOG_DEBUG("Created pixel shader binary with size {}", ps.size());

        // Create pipeline layout
        pipeline_layout_ = render_device_->create_pipeline_layout({});

        // Create graphics pipeline
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

        // Create vertex buffer
        vertex_buffer_ = render_device_->create_buffer({.size = sizeof(vertices), .usage = BufferUsage::Vertex, .cpu_visible = true});

        // Upload vertex data
        render_device_->memcpy(vertex_buffer_, vertices.data(), sizeof(vertices));
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
        // Reset command allocator
        command_allocator_->reset();

        // Begin command list recording
        command_list_->begin();

        // Acquire render target from swapchain
        const auto render_target = swapchain_->acquire_render_target();

        // Get the swapchain image associated with render target
        const auto image = swapchain_->current_image();

        // Transition swapchain image to a render target
        command_list_->transition_barrier({
            .image = image,
            .before = ResourceState::Unknown,
            .after = ResourceState::RenderTarget,
        });

        // Begin rendering to render_target and clear it
        command_list_->begin_rendering({
            .render_targets = {{
                RenderAttachment{
                    .render_target = render_target,
                    .clear_color = {1.f, 0.f, 1.f, 1.f},
                },
            }},
            .render_area = {
                .width = 800,
                .height = 600,
            },
        });

        // Set the graphics pipeline
        command_list_->set_pipeline({.pipeline = graphics_pipeline_});

        // Set the viewport
        command_list_->set_viewports({
            .viewports = {{
                Viewport{
                    .x = 0.f,
                    .y = 0.f,
                    .width = 800.f,
                    .height = 600.f,
                    .min_depth = 0.f,
                    .max_depth = 1.f,
                },
            }},
        });

        // Set scissors
        command_list_->set_scissors({
            .scissors = {{
                Rect2D{
                    .x = 0,
                    .y = 0,
                    .width = 800,
                    .height = 600,
                },
            }},
        });

        // Set vertex buffer
        command_list_->set_vertex_buffers({
            .start_buffer = 0,
            .vertex_buffers = {{
                BufferView{
                    .buffer = vertex_buffer_,
                    .stride = sizeof(Vertex),
                },
            }},
        });

        // Make draw call
        command_list_->draw_instanced({.vertex_count = 3, .instance_count = 1, .start_vertex = 0, .start_instance = 0});

        // End rendering
        command_list_->end_rendering();

        // Transition image to present image
        command_list_->transition_barrier({
            .image = image,
            .before = ResourceState::RenderTarget,
            .after = ResourceState::Present,
        });

        // End command list recording
        command_list_->end();

        // Submit command list
        command_queue_->submit({{command_list_.get()}}, fence_);

        // Present swapchain with vsync == true
        swapchain_->present(true);

        // Wait for frame to finish
        render_device_->wait_for_fence(fence_);
    }

    Window window_;
    std::unique_ptr<RenderBackend> render_backend_;
    std::unique_ptr<RenderDevice> render_device_;
    std::unique_ptr<CommandQueue> command_queue_;
    std::unique_ptr<Swapchain> swapchain_;
    std::unique_ptr<CommandAllocator> command_allocator_;
    std::unique_ptr<CommandList> command_list_;
    FenceHandle fence_;
    PipelineLayoutHandle pipeline_layout_;
    PipelineHandle graphics_pipeline_;
    BufferHandle vertex_buffer_;
};

std::unique_ptr<Application> orion_main(std::span<const char* const> args)
{
    return std::make_unique<SandboxApp>();
}
