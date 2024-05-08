#include "orion-renderer/quad_renderer.h"

#include "orion-renderer/shader.h"
#include "orion-renderer/texture.h"

#include "orion-renderapi/render_device.h"

#include "orion-math/matrix/transformation.h"

#include <algorithm>
#include <span>

namespace orion
{
    namespace
    {
        struct QuadGPUData {
            Matrix4_f transform;
            Vector4_f color;
        };

        std::vector<QuadGPUData> gpu_data;

        bool quad_compare(const QuadData& lhs, const QuadData& rhs)
        {
            return lhs.texture_index < rhs.texture_index;
        }
    } // namespace

    QuadRenderer::QuadRenderer(const QuadRendererDesc& desc)
        : device_(desc.device)
        , texture_manager_(desc.texture_manager)
        , descriptor_layout_(device_->to_unique(create_descriptor_layout()))
        , pipeline_layout_(device_->to_unique(create_pipeline_layout()))
        , pipeline_(device_->to_unique(create_pipeline(desc.shader_manager, desc.render_pass)))
        , descriptor_pool_(device_->to_unique(create_descriptor_pool()))
        , frames_([this] { return create_frame_data(); })
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
        if (quads_.empty()) {
            return;
        }

        // Sort quads based on texture indices
        std::sort(quads_.begin(), quads_.end(), quad_compare);

        // Generate gpu data
        gpu_data.resize(quads_.size());
        std::ranges::transform(quads_, gpu_data.begin(), [](const QuadData& quad) {
            return QuadGPUData{
                .transform = Matrix4_f::identity() * translation(quad.position) * rotation_z(quad.rotation) * scaling(vec3(quad.scale, 1.f)),
                .color = quad.color,
            };
        });

        // Upload data to gpu
        auto& frame = frames_.get(render_context.frame_index());
        auto& buffer = frame.quad_buffer;
        buffer.upload_grow(std::as_bytes(std::span{gpu_data}));

        // Update buffer descriptor
        const auto buffer_descriptor_desc = BufferDescriptorDesc{
            .buffer_handle = buffer.handle(),
            .region = {
                .size = buffer.size(),
                .offset = 0,
            },
        };
        const auto descriptor_write = DescriptorWrite{
            .binding = 0,
            .descriptor_type = DescriptorType::StorageBuffer,
            .buffers = {&buffer_descriptor_desc, 1},
        };
        device_->write_descriptor(frame.buffer_descriptor, descriptor_write);

        auto* command_list = render_context.command_list();
        // Bind quad pipeline
        command_list->bind_pipeline({.pipeline = pipeline_.get(), .bind_point = PipelineBindPoint::Graphics});
        // Bind the quad buffer descriptor
        const auto buffer_descriptor = frame.buffer_descriptor;
        command_list->bind_descriptor({.bind_point = PipelineBindPoint::Graphics, .pipeline_layout = pipeline_layout_.get(), .index = 0, .descriptor = buffer_descriptor});
        // Bind the texture descriptor
        command_list->bind_descriptor({.bind_point = PipelineBindPoint::Graphics, .pipeline_layout = pipeline_layout_.get(), .index = 1, .descriptor = texture_manager_->descriptor()});

        auto iter = quads_.begin();
        auto vertex_offset = 0u;
        while (iter != quads_.end()) {
            // Set current texture index as push constant
            texture_index_t texture_index = iter->texture_index;
            command_list->push_constants({
                .pipeline_layout = pipeline_layout_.get(),
                .shader_stages = ShaderStageFlags::Pixel,
                .offset = 0,
                .size = sizeof(texture_index_t),
                .values = &texture_index,
            });

            // Calculate number quads/vertices to draw
            auto next = std::upper_bound(iter, quads_.end(), texture_index, [](texture_index_t index, const QuadData& quad) { return index < quad.texture_index; });
            const auto quad_count = std::distance(iter, next);
            const auto vertex_count = static_cast<std::uint32_t>(quad_count * quad_vertex_count);

            // Make draw call
            command_list->draw({
                .vertex_count = vertex_count,
                .instance_count = 1,
                .first_vertex = vertex_offset,
                .first_instance = 1,
            });

            // Add vertex offset
            vertex_offset += vertex_count;

            // Update iterator
            iter = next;
        }
    }

    DescriptorLayoutHandle QuadRenderer::create_descriptor_layout() const
    {
        return device_->create_descriptor_layout({
            .bindings = {{
                DescriptorBindingDesc{
                    .type = DescriptorType::StorageBuffer,
                    .shader_stages = ShaderStageFlags::Vertex,
                    .count = 1,
                },
            }},
        });
    }

    PipelineLayoutHandle QuadRenderer::create_pipeline_layout() const
    {
        return device_->create_pipeline_layout({
            .descriptors = {{descriptor_layout_.get(), texture_manager_->descriptor_layout()}},
            .push_constants = {{
                PushConstantDesc{.size = sizeof(texture_index_t), .shader_stages = ShaderStageFlags::Pixel},
            }},
        });
    }

    PipelineHandle QuadRenderer::create_pipeline(ShaderManager* shader_manager, RenderPassHandle render_pass) const
    {
        auto shader = shader_manager->load_shader_effect("quad");
        return device_->create_graphics_pipeline({
            .shaders = shader.shader_stages(),
            .vertex_bindings = {},
            .pipeline_layout = pipeline_layout_.get(),
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

    DescriptorPoolHandle QuadRenderer::create_descriptor_pool() const
    {
        return device_->create_descriptor_pool({
            .max_descriptors = 2,
            .sizes = {{
                DescriptorPoolSize{
                    .type = DescriptorType::StorageBuffer,
                    .count = 2,
                },
            }},
        });
    }

    QuadRenderer::FrameData QuadRenderer::create_frame_data() const
    {
        return FrameData{
            .quad_buffer = {device_, 0, GPUBufferUsageFlags::StorageBuffer},
            .buffer_descriptor = device_->create_descriptor(descriptor_layout_.get(), descriptor_pool_.get()),
        };
    }
} // namespace orion
