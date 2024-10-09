#include <orion/application.h>
#include <orion/window.h>

#include <orion/renderapi/render_backend.h>
#include <orion/renderapi/shader.h>

#include <orion/math/vector/vector3.h>
#include <orion/math/vector/vector4.h>

#include <spdlog/spdlog.h>

#include <array>

using namespace orion;

constexpr auto vertex_shader = R"hlsl(
float4 main(float3 position : POSITION) : SV_Position
{
    return float4(position, 1.0);
})hlsl";

constexpr auto pixel_shader = R"hlsl(
cbuffer Constants {
    float4 color;
};

float4 main() : SV_Target
{
    return color;
}
)hlsl";

struct Vertex {
    Vector3f position;
};

constexpr auto vertices = std::array{
    Vertex{.position = {-.5f, .5f, 0.f}},
    Vertex{.position = {.5f, .5f, 0.f}},
    Vertex{.position = {.5f, -.5f, 0.f}},
    Vertex{.position = {-.5f, -.5f, 0.f}},
};

constexpr auto indices = std::array{0u, 1u, 2u, 2u, 3u, 0u};

class SandboxApp final : public Application
{
public:
    SandboxApp()
        : window_({.title = "Sandbox App", .width = 800, .height = 600})
    {
        // Create render backend
        render_backend_ = RenderBackend::create_builtin_vulkan();

        // Create render device
        render_device_ = render_backend_->create_device(0);

        // Create command queue
        command_queue_ = render_device_->create_command_queue();

        // Create swapchain
        swapchain_ = render_device_->create_swapchain({.window = &window_, .queue = command_queue_.get(), .width = 800, .height = 600, .image_count = 2, .image_format = Format::B8G8R8A8_Unorm});

        // Create command allocator
        command_allocator_ = render_device_->create_command_allocator({});

        // Create command list
        command_list_ = render_device_->create_command_list({.command_allocator = command_allocator_.get()});

        // Create descriptor pool
        descriptor_pool_ = render_device_->create_descriptor_pool({
            .max_descriptor_sets = 1,
            .descriptor_sizes = {{
                DescriptorPoolSize{.type = DescriptorType::ConstantBuffer, .count = 1},
            }},
        });

        // Create frame fence
        fence_ = render_device_->create_fence({.signaled = false});

        // Compile shaders
        auto compiler = render_device_->create_shader_compiler();
        const auto vs = compiler->compile({.source = vertex_shader, .type = ShaderType::Vertex});
        const auto ps = compiler->compile({.source = pixel_shader, .type = ShaderType::Pixel});
        if (vs.empty() || ps.empty()) {
            orion_abort("Failed to compile shaders");
        }
        SPDLOG_DEBUG("Created vertex shader binary with size {}", vs.size());
        SPDLOG_DEBUG("Created pixel shader binary with size {}", ps.size());

        // Create descriptor set layout
        descriptor_set_layout_ = render_device_->create_descriptor_set_layout({
            .bindings = {{
                DescriptorSetBindingDesc{.type = DescriptorType::ConstantBuffer, .size = 1},
            }},
        });

        // Create pipeline layout
        pipeline_layout_ = render_device_->create_pipeline_layout({
            .descriptor_set_layouts = {{descriptor_set_layout_}},
        });

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
        vertex_buffer_ = render_device_->create_buffer({.size = sizeof(vertices), .usage = BufferUsage::VertexBuffer, .cpu_visible = true});

        // Upload vertex data
        render_device_->memcpy(vertex_buffer_, vertices.data(), sizeof(vertices));

        // Create index buffer
        index_buffer_ = render_device_->create_buffer({.size = sizeof(indices), .usage = BufferUsage::IndexBuffer, .cpu_visible = true});

        // Upload index data
        render_device_->memcpy(index_buffer_, indices.data(), sizeof(indices));

        // Create descriptor set
        descriptor_set_ = render_device_->create_descriptor_set({.layout = descriptor_set_layout_, .pool = descriptor_pool_});

        // Create constant buffer
        constant_buffer_ = render_device_->create_buffer({.size = sizeof(Vector4f), .usage = BufferUsage::ConstantBuffer, .cpu_visible = true});

        // Create constant buffer view
        render_device_->create_constant_buffer_view({
            .buffer = constant_buffer_,
            .offset = 0,
            .size = sizeof(Vector4f),
            .descriptor_set = descriptor_set_,
            .descriptor_binding = 0,
        });
    }

private:
    void on_update() override
    {
        WindowEvent event;
        while ((event = window_.poll_event())) {
            SPDLOG_TRACE("{}", event);
            if (event.is<OnWindowClose>()) {
                orion_exit();
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
                VertexBufferView{.buffer = vertex_buffer_, .stride = sizeof(Vertex)},
            }},
        });

        // Set index buffer
        command_list_->set_index_buffer({.buffer = index_buffer_, .index_type = IndexType::U32});

        // Set color and descriptor set
        static std::uint32_t color = 0;
        const auto color_f = color / 255.f;
        const auto color_vf = Vector4f{color_f, color_f, color_f, 1.0f};
        render_device_->memcpy(constant_buffer_, &color_vf, sizeof(color_vf));
        color = (color + 1) & 0xff; // (color + 1) % 255
        command_list_->set_descriptor_set({.set = 0, .descriptor_set = descriptor_set_, .pipeline_layout = pipeline_layout_});

        // Make draw call
        command_list_->draw_indexed_instanced({.index_count = 6, .instance_count = 1, .first_index = 0, .first_vertex = 0, .first_instance = 0});

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
    DescriptorSetLayoutHandle descriptor_set_layout_;
    PipelineLayoutHandle pipeline_layout_;
    PipelineHandle graphics_pipeline_;
    BufferHandle vertex_buffer_;
    BufferHandle index_buffer_;
    DescriptorPoolHandle descriptor_pool_;
    DescriptorSetHandle descriptor_set_;
    BufferHandle constant_buffer_;
};

std::unique_ptr<Application> orion_main(std::span<const char* const> args)
{
    return std::make_unique<SandboxApp>();
}
