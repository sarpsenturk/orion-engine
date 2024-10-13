#include <orion/application.hpp>
#include <orion/window.hpp>

#include <orion/renderapi/render_backend.hpp>
#include <orion/renderapi/shader.hpp>

#include <orion/renderer/renderer.hpp>

#include <orion/math/matrix/matrix4.hpp>
#include <orion/math/matrix/projection.hpp>
#include <orion/math/vector/vector3.hpp>
#include <orion/math/vector/vector4.hpp>

#include <spdlog/spdlog.h>

#include <array>

using namespace orion;

constexpr auto vertex_shader = R"hlsl(
cbuffer Scene : register(b0) {
    float4x4 projection;
};

float4 main(float3 position : POSITION) : SV_Position
{
    return mul(projection, float4(position, 1.0));
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
    Vertex{.position = {0.f, .5f, -.1f}},
    Vertex{.position = {.5f, -.5f, -.1f}},
    Vertex{.position = {-.5f, -.5f, -.1f}},
};

constexpr auto indices = std::array{0u, 1u, 2u};

class SandboxApp final : public Application
{
public:
    SandboxApp()
        : window_({.title = "Sandbox App", .width = 800, .height = 600})
        , renderer_({.render_backend = RenderBackend::create_builtin_vulkan(), .width = 800, .height = 600})
    {
        // Get render device from renderer
        auto* render_device = renderer_.render_device();

        // Get command queue from renderer
        auto* graphics_queue = renderer_.graphics_queue();

        // Create swapchain
        swapchain_ = render_device->create_swapchain({.window = &window_, .queue = graphics_queue, .width = 800, .height = 600, .image_count = 2, .image_format = Format::B8G8R8A8_Unorm});

        // Create descriptor pool
        descriptor_pool_ = render_device->create_descriptor_pool({
            .max_descriptor_sets = 1,
            .descriptor_sizes = {{
                DescriptorPoolSize{.type = DescriptorType::ConstantBuffer, .count = 1},
            }},
        });

        // Compile shaders
        auto compiler = render_device->create_shader_compiler();
        const auto vs = compiler->compile({.source = vertex_shader, .type = ShaderType::Vertex});
        const auto ps = compiler->compile({.source = pixel_shader, .type = ShaderType::Pixel});
        if (vs.empty() || ps.empty()) {
            orion_abort("Failed to compile shaders");
        }
        SPDLOG_DEBUG("Created vertex shader binary with size {}", vs.size());
        SPDLOG_DEBUG("Created pixel shader binary with size {}", ps.size());

        // Create descriptor set layout
        descriptor_set_layout_ = render_device->create_descriptor_set_layout({
            .bindings = {{
                DescriptorSetBindingDesc{.type = DescriptorType::ConstantBuffer, .size = 1},
            }},
        });

        // Create pipeline layout
        pipeline_layout_ = render_device->create_pipeline_layout({
            .descriptor_set_layouts = {{descriptor_set_layout_}},
        });

        // Create graphics pipeline
        graphics_pipeline_ = render_device->create_graphics_pipeline({
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
        vertex_buffer_ = render_device->create_buffer({.size = sizeof(vertices), .usage = BufferUsage::VertexBuffer, .cpu_visible = true});

        // Upload vertex data
        render_device->memcpy(vertex_buffer_, vertices.data(), sizeof(vertices));

        // Create index buffer
        index_buffer_ = render_device->create_buffer({.size = sizeof(indices), .usage = BufferUsage::IndexBuffer, .cpu_visible = true});

        // Upload index data
        render_device->memcpy(index_buffer_, indices.data(), sizeof(indices));

        // Create descriptor set
        descriptor_set_ = render_device->create_descriptor_set({.layout = descriptor_set_layout_, .pool = descriptor_pool_});

        // Create constant buffer
        constant_buffer_ = render_device->create_buffer({.size = sizeof(Matrix4f), .usage = BufferUsage::ConstantBuffer, .cpu_visible = true});

        // Upload projection matrix
        const auto projection = orthographic_rh(-4.f, 4.f, -3.f, 3.f, 0.1f, 1.f);
        render_device->memcpy(constant_buffer_, &projection, sizeof(Matrix4f));

        // Create constant buffer view
        render_device->create_constant_buffer_view({
            .buffer = constant_buffer_,
            .offset = 0,
            .size = sizeof(Matrix4f),
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
        // Begin a new frame
        renderer_.begin_frame();

        // Get command list from renderer
        auto* draw_command = renderer_.draw_command();

        // Set the graphics pipeline
        draw_command->set_pipeline({.pipeline = graphics_pipeline_});

        // Set the viewport
        draw_command->set_viewports({
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
        draw_command->set_scissors({
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
        draw_command->set_vertex_buffers({
            .start_buffer = 0,
            .vertex_buffers = {{
                VertexBufferView{.buffer = vertex_buffer_, .stride = sizeof(Vertex)},
            }},
        });

        // Set index buffer
        draw_command->set_index_buffer({.buffer = index_buffer_, .index_type = IndexType::U32});

        // Set color and descriptor set
        draw_command->set_descriptor_set({.set = 0, .descriptor_set = descriptor_set_, .pipeline_layout = pipeline_layout_});

        // Make draw call
        draw_command->draw_indexed_instanced({.index_count = 3, .instance_count = 1, .first_index = 0, .first_vertex = 0, .first_instance = 0});

        // End frame
        renderer_.end_frame();

        // Present results to swapchain
        renderer_.present(swapchain_.get(), 800, 600);
    }

    Window window_;
    Renderer renderer_;
    std::unique_ptr<Swapchain> swapchain_;
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
