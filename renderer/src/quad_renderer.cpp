#include "orion-renderer/quad_renderer.h"

#include "orion-renderer/shader.h"

#include "orion-renderapi/render_device.h"

namespace orion
{
    QuadRenderer::QuadRenderer(RenderDevice* device, ShaderManager* shader_manager, RenderPassHandle render_pass)
        : device_(device)
        , pipeline_(create_pipeline(shader_manager, render_pass))
        , frames_([this] { return FrameData{.quad_buffer = {device_, GPUBufferUsageFlags::StorageBuffer}}; })
    {
    }

    void QuadRenderer::begin()
    {
        quads_.clear();
    }

    void QuadRenderer::add(const QuadData& quad)
    {
        quads_.push_back(quad);
    }

    void QuadRenderer::flush(CommandList* command_list)
    {
        command_list->bind_pipeline({.pipeline = pipeline_, .bind_point = PipelineBindPoint::Graphics});
        command_list->draw({.vertex_count = static_cast<std::uint32_t>(vertex_count()), .instance_count = 1, .first_vertex = 0, .first_instance = 0});
    }

    PipelineHandle QuadRenderer::create_pipeline(ShaderManager* shader_manager, RenderPassHandle render_pass) const
    {
        auto shader = shader_manager->load_shader_effect("quad");
        return device_->create_graphics_pipeline({
            .shaders = shader.shader_stages(),
            .vertex_bindings = {},
            .pipeline_layout = {},
            .input_assembly = {.topology = PrimitiveTopology::TriangleList},
            .rasterization = {.front_face = FrontFace::ClockWise},
            .color_blend = {
                .attachments = {{
                    BlendAttachmentDesc{
                        .enable_blend = true,
                        .src_blend = BlendFactor::One,
                        .dst_blend = BlendFactor::Zero,
                        .blend_op = BlendOp::Add,
                        .color_component_flags = ColorComponentFlags::All,
                    },
                }},
            },
            .render_pass = render_pass,
        });
    }
} // namespace orion
