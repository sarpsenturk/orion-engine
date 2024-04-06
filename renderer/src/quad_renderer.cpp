#include "orion-renderer/quad_renderer.h"

#include "orion-renderer/shader.h"

#include "orion-renderapi/render_device.h"

#include <span>

namespace orion
{
    QuadRenderer::QuadRenderer(RenderDevice* device, ShaderManager* shader_manager, RenderPassHandle render_pass)
        : device_(device)
        , descriptor_layout_(create_descriptor_layout())
        , pipeline_layout_(create_pipeline_layout())
        , pipeline_(create_pipeline(shader_manager, render_pass))
        , frames_([this] { return FrameData{.quad_buffer = {device_, 0, GPUBufferUsageFlags::StorageBuffer}, .descriptor = device_->create_descriptor(descriptor_layout_)}; })
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

    void QuadRenderer::flush(const RenderContext& render_context)
    {
        auto& frame = frames_.get(render_context.frame_index());
        auto& buffer = frame.quad_buffer;

        buffer.upload_grow(std::as_bytes(std::span{quads_}));
        device_->write_descriptor(frame.descriptor, frame.quad_buffer.descriptor_binding(0));

        auto* command_list = render_context.command_list();
        command_list->bind_pipeline({.pipeline = pipeline_, .bind_point = PipelineBindPoint::Graphics});
        const auto descriptor = frame.descriptor;
        command_list->bind_descriptor({.bind_point = PipelineBindPoint::Graphics, .pipeline_layout = pipeline_layout_, .index = 0, .descriptor = descriptor});
        command_list->draw({.vertex_count = static_cast<std::uint32_t>(vertex_count()), .instance_count = 1, .first_vertex = 0, .first_instance = 0});
    }

    DescriptorLayoutHandle QuadRenderer::create_descriptor_layout() const
    {
        return device_->create_descriptor_layout({
            .bindings = {{
                DescriptorBindingDesc{
                    .type = BindingType::StorageBuffer,
                    .shader_stages = ShaderStageFlags::Vertex,
                    .count = 1,
                },
            }},
        });
    }

    PipelineLayoutHandle QuadRenderer::create_pipeline_layout() const
    {
        return device_->create_pipeline_layout({.descriptors = {{descriptor_layout_}}});
    }

    PipelineHandle QuadRenderer::create_pipeline(ShaderManager* shader_manager, RenderPassHandle render_pass) const
    {
        auto shader = shader_manager->load_shader_effect("quad");
        return device_->create_graphics_pipeline({
            .shaders = shader.shader_stages(),
            .vertex_bindings = {},
            .pipeline_layout = pipeline_layout_,
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
