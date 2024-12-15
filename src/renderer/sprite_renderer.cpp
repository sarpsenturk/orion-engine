#include "orion/renderer/sprite_renderer.hpp"

#include "orion/renderapi/render_command.hpp"
#include "orion/renderapi/render_device.hpp"

#include "orion/renderer/renderer.hpp"

#include "orion/math/matrix/transformation.hpp"

#include <stdexcept>
static constexpr auto vertex_shader = R"hlsl(
static const float3 corners[] = {
    float3(-0.5, 0.5, 0.0),
    float3(0.5, 0.5, 0.0),
    float3(0.5, -0.5, 0.0),
    float3(-0.5, -0.5, 0.0),
};

cbuffer Scene : register(b0) {
    float4x4 view_projection;
};

struct SpriteData {
    float4x4 model;
};
StructuredBuffer<SpriteData> sprites;

float4 main(uint id : SV_VertexID) : SV_Position
{
    uint vertex_idx = id % 6;
    uint corner_idx = vertex_idx < 3 ? vertex_idx : (vertex_idx - 1) % 4;
    uint sprite_idx = id / 6;
    SpriteData sprite = sprites[sprite_idx];
    return mul(view_projection, mul(sprite.model, float4(corners[corner_idx], 1.0)));
})hlsl";

static constexpr auto pixel_shader = R"hlsl(
float4 main() : SV_Target
{
    return float4(1.0, 1.0, 1.0, 1.0);
}
)hlsl";

namespace orion
{
    SpriteRenderer::SpriteRenderer(const SpriteRendererDesc& desc)
    {
        auto* render_device = desc.render_device;

        // Create bind layouts
        bind_layout_ = render_device->create_bind_group_layout({
            .bindings = {{
                DescriptorSetBindingDesc{.type = DescriptorType::ConstantBuffer, .size = 1},
                DescriptorSetBindingDesc{.type = DescriptorType::StructuredBuffer, .size = 1},
            }},
        });

        // Create pipeline layout
        pipeline_layout_ = render_device->create_pipeline_layout({.bind_group_layouts = {{bind_layout_}}});

        // Compile shaders for pipeline
        auto shader_compiler = render_device->create_shader_compiler();
        const auto vs_binary = shader_compiler->compile({.source = vertex_shader, .type = ShaderType::Vertex});
        const auto ps_binary = shader_compiler->compile({.source = pixel_shader, .type = ShaderType::Pixel});
        if (vs_binary.empty() || ps_binary.empty()) {
            throw std::runtime_error("Renderer2D: failed to compile shaders");
        }

        // Create pipeline
        pipeline_ = render_device->create_graphics_pipeline({
            .pipeline_layout = pipeline_layout_,
            .vertex_shader = vs_binary,
            .pixel_shader = ps_binary,
            .vertex_attributes = {},
            .primitive_topology = PrimitiveTopology::Triangle,
            .rasterizer = {
                .fill_mode = FillMode::Solid,
                .cull_mode = CullMode::Back,
                .front_face = FrontFace::ClockWise,
            },
            .blend = {
                .render_targets = {{
                    RenderTargetBlendDesc{
                        .blend_enable = false,
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

        // Create constant buffer for SceneData and create constant buffer view
        constant_buffer_ = render_device->create_buffer({.size = sizeof(SceneData), .usage_flags = BufferUsageFlags::ConstantBuffer, .cpu_visible = true});

        // Create structured buffer for SpriteData and create robuffer_view
        // TODO: Consider making this not CPU visible and use a staging buffer for this
        sprite_buffer_ = render_device->create_buffer({.size = sprite_buffer_size, .usage_flags = BufferUsageFlags::StructuredBuffer, .cpu_visible = true});

        // Create bind group
        bind_group_ = render_device->create_bind_group({
            .layout = bind_layout_,
            .buffers = {{
                BufferBindingDesc{.binding = 0, .buffer = constant_buffer_, .type = DescriptorType::ConstantBuffer, .offset = 0, .size = sizeof(SceneData)},
                BufferBindingDesc{.binding = 1, .buffer = sprite_buffer_, .type = DescriptorType::StructuredBuffer, .offset = 0, .size = sprite_buffer_size},
            }},
        });

        // Allocate CPU memory for sprites
        sprites_.reserve(max_batch_size);

        // Setup camera
        const auto view_width = desc.screen_width / desc.camera_size / 2;
        const auto view_height = desc.screen_height / desc.camera_size / 2;
        camera_.ortho(-view_width, view_width, -view_height, view_height, 0.f, 1.f);

        // Initialize view_projection matrix
        const auto scene_data = SceneData{.view_projection = camera_.view_projection()};
        render_device->memcpy(constant_buffer_, &scene_data, sizeof(SceneData));
    }

    void SpriteRenderer::draw(const Vector3f& position, const Vector2f& size)
    {
        const auto model = Matrix4f::identity() * scale(vec3(size, 1.f)) * translate(position);
        sprites_.push_back({.model = model});
    }

    void SpriteRenderer::submit(Renderer* renderer)
    {
        // Get draw command list
        auto* command_list = renderer->draw_command();

        // Set pipeline
        command_list->set_pipeline({.pipeline = pipeline_, .layout = pipeline_layout_});

        // Set viewport
        command_list->set_viewports({
            .start_viewport = 0,
            .viewports = {{
                Viewport{
                    .x = 0.f,
                    .y = 0.f,
                    .width = static_cast<float>(renderer->render_width()),
                    .height = static_cast<float>(renderer->render_height()),
                    .min_depth = 0.f,
                    .max_depth = 1.f,
                },
            }},
        });

        // Set scissor
        command_list->set_scissors({
            .start_scissor = 0,
            .scissors = {{
                Rect2D{
                    .x = 0,
                    .y = 0,
                    .width = static_cast<std::int32_t>(renderer->render_width()),
                    .height = static_cast<std::int32_t>(renderer->render_height()),
                },
            }},
        });

        // Update sprite buffer
        {
            auto* render_device = renderer->render_device();
            render_device->memcpy(sprite_buffer_, sprites_.data(), sizeof(SpriteData) * sprites_.size());
        }

        // Set bind group
        command_list->set_bind_group({.index = 0, .bind_group = bind_group_, .pipeline_layout = pipeline_layout_});

        // Draw call
        command_list->draw_instanced({.vertex_count = vertex_count(), .instance_count = 1, .start_vertex = 0, .start_instance = 0});

        // Reset sprites
        sprites_.clear();
    }
} // namespace orion
