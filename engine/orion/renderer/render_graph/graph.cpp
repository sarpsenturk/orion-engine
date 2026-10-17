#include "graph.hpp"

#include "orion/core/assert.hpp"

#include <algorithm>
#include <initializer_list>
#include <ranges>

namespace orion
{
    static constexpr auto write_mask = gpu::Access::ShaderWrite |
                                       gpu::Access::ColorAttachmentWrite |
                                       gpu::Access::DepthStencilAttachmentWrite |
                                       gpu::Access::TransferWrite |
                                       gpu::Access::HostWrite;
    static bool is_write(const RGResourceState& state)
    {
        return (state.access & write_mask) != gpu::Access::None;
    }

    static bool is_layout_transition(const RGResourceState& before, const RGResourceState& after)
    {
        return before.layout != after.layout;
    }

    void RenderGraph::compile()
    {
        std::vector<RGResourceState> resource_states(m_resources.size());
        // Initialize resource states based on imported resources
        std::ranges::transform(m_resources, resource_states.begin(), [](const Resource& resource) {
            if (resource.import_desc.has_value()) {
                return std::visit([](const auto& import_desc) { return import_desc.initial_state; }, *resource.import_desc);
            } else {
                return RGResourceState{};
            }
        });

        // Walk through the pass declarations
        for (std::uint32_t pass_index = 0u; pass_index < m_pass_decls.size(); ++pass_index) {
            auto& pass_decl = m_pass_decls[pass_index];
            ORION_ASSERT((pass_decl.type == RenderPassType::Graphics) == (!pass_decl.color_attachments.empty()) && "Graphics passes must have at least one color attachment");
            auto& compiled_pass = m_compiled_passes.emplace_back(pass_index);

            // 1. Walk through the resources accessed by this pass and insert barriers if needed
            compile_pass_emit_barriers(compiled_pass, pass_decl, resource_states);

            // 2. Resolve the attachments for the pass
            compile_pass_resolve_attachments(compiled_pass, pass_decl);
        }

        // Walk through final resource states and insert barriers to transition to final states if needed
        for (std::uint32_t resource_index = 0u; resource_index < m_resources.size(); ++resource_index) {
            const auto& resource = m_resources[resource_index];
            if (resource.import_desc.has_value()) {
                const auto& current_state = resource_states[resource_index];
                const auto final_state    = std::visit([](const auto& import_desc) { return import_desc.final_state; }, *resource.import_desc);
                if (is_write(current_state) || is_layout_transition(current_state, final_state)) {
                    if (auto* texture = std::get_if<GpuTexture>(&m_gpu_resources[resource_index])) {
                        m_final_image_barriers.push_back(make_image_barrier(texture->image, current_state, final_state));
                    } else {
                        auto& buffer = std::get<GpuBuffer>(m_gpu_resources[resource_index]);
                        m_final_buffer_barriers.push_back(make_buffer_barrier(buffer.buffer, current_state, final_state));
                    }
                }
            }
        }
    }

    void RenderGraph::execute(gpu::CommandBuffer* command_buffer)
    {
        // Execute each pass in order
        for (const auto& pass : m_compiled_passes) {
            const auto& pass_decl = m_pass_decls[pass.pass_index];

            // Set debug name for the pass
            gpu::debug_marker_begin(command_buffer, pass_decl.name.c_str());

            // Insert barriers for the pass
            gpu::barrier(command_buffer, gpu::Barrier{pass.image_barriers, pass.buffer_barriers});

            // Execute the pass
            switch (pass_decl.type) {
                case RenderPassType::Graphics:
                    execute_graphics_pass(command_buffer, pass_decl, pass);
                    break;
            }

            gpu::debug_marker_end(command_buffer);
        }

        // Insert final layout transition barriers
        gpu::barrier(command_buffer, gpu::Barrier{m_final_image_barriers, m_final_buffer_barriers});
    }

    void RenderGraph::reset()
    {
        // Clear all internal data structures to prepare for a new frame
        m_pass_decls.clear();
        m_compiled_passes.clear();
        m_resources.clear();
        m_gpu_resources.clear();
        m_final_image_barriers.clear();
        m_final_buffer_barriers.clear();
    }

    RGTexture RenderGraph::import_texture(
        std::string name,
        const RGTextureDesc& desc,
        const RGTextureImportDesc& import)
    {
        ORION_ASSERT(import.final_state.layout != gpu::ImageLayout::Undefined && "Cannot transition to undefined layout");

        const auto index = static_cast<std::uint32_t>(m_resources.size());
        m_resources.emplace_back(
            std::move(name),
            desc,
            import,
            std::initializer_list{ResourceVersion{.writer_pass = ~0u, .contents_defined = true}});
        m_gpu_resources.emplace(index, GpuTexture{import.image, import.image_view});
        return RGTexture{index, 0u};
    }

    RGBuffer RenderGraph::import_buffer(
        std::string name,
        const RGBufferDesc& desc,
        const RGBufferImportDesc& import)
    {
        const auto index = static_cast<std::uint32_t>(m_resources.size());
        m_resources.emplace_back(
            std::move(name),
            desc,
            import,
            std::initializer_list{ResourceVersion{.writer_pass = ~0u, .contents_defined = true}});
        m_gpu_resources.emplace(index, GpuBuffer{import.buffer});
        return RGBuffer{index, 0u};
    }

    void RenderGraph::compile_pass_emit_barriers(
        RenderPass& compiled_pass,
        const RenderPassDecl& pass_decl,
        std::vector<RGResourceState>& resource_states)
    {
        for (const auto& access : pass_decl.resource_accesses) {
            auto& state = resource_states[access.resource_index];
            // There are 3 hazards to consider:
            // 1. Write after Read (WAR):
            //  If the current pass is writing to a resource that was read by a previous pass,
            //  we need to insert a barrier to ensure the read completes before the write.
            // 2. Read after Write (RAW):
            //  If the current pass is reading from a resource that was written by a previous pass,
            //  we need to insert a barrier to ensure the write is complete & visible before the read.
            // 3. Write after Write (WAW):
            //  If the current pass is writing to a resource that was written by a previous pass,
            //  we need to insert a barrier to ensure the first write completes before the second write.
            if (is_write(state) || is_write(access.state) || is_layout_transition(state, access.state)) {
                // Insert a barrier to ensure the previous access is complete before the current access
                if (auto* texture = std::get_if<GpuTexture>(&m_gpu_resources[access.resource_index])) {
                    compiled_pass.image_barriers.push_back(make_image_barrier(texture->image, state, access.state));
                } else {
                    auto& buffer = std::get<GpuBuffer>(m_gpu_resources[access.resource_index]);
                    compiled_pass.buffer_barriers.push_back(make_buffer_barrier(buffer.buffer, state, access.state));
                }
            }

            // Update the resource state to the current access state
            state = access.state;
        }
    }

    void RenderGraph::compile_pass_resolve_attachments(
        RenderPass& compiled_pass,
        const RenderPassDecl& pass_decl)
    {
        if (pass_decl.type == RenderPassType::Graphics) {
            compiled_pass.color_attachments.resize(pass_decl.color_attachments.size());
            std::ranges::transform(pass_decl.color_attachments, compiled_pass.color_attachments.begin(), [this](const AttachmentRef& attachment) {
                auto& texture = std::get<GpuTexture>(m_gpu_resources[attachment.resource_index]);
                return gpu::RenderPassAttachment{
                    .image_view  = texture.view,
                    .load_op     = attachment.load_op,
                    .store_op    = attachment.store_op,
                    .clear_value = attachment.clear_value,
                };
            });
            if (pass_decl.depth_attachment.has_value()) {
                const auto& attachment         = *pass_decl.depth_attachment;
                auto& texture                  = std::get<GpuTexture>(m_gpu_resources[attachment.resource_index]);
                compiled_pass.depth_attachment = gpu::RenderPassAttachment{
                    .image_view  = texture.view,
                    .load_op     = attachment.load_op,
                    .store_op    = attachment.store_op,
                    .clear_value = attachment.clear_value,
                };
            }
        }
    }

    void RenderGraph::execute_graphics_pass(
        gpu::CommandBuffer* command_buffer,
        const RenderPassDecl& pass_decl,
        const RenderPass& pass)
    {
        // Begin the render pass
        gpu::begin_render_pass(
            command_buffer,
            gpu::RenderPass{
                .color_attachments = pass.color_attachments,
            });

        // Execute the pass callback
        auto context = RenderPassContext(this, command_buffer);
        pass_decl.execute(context);

        // End the render pass
        gpu::end_render_pass(command_buffer);
    }

    gpu::ImageBarrier RenderGraph::make_image_barrier(
        gpu::Image* image,
        const RGResourceState& before,
        const RGResourceState& after) const
    {
        return gpu::ImageBarrier{
            .image      = image,
            .src_stages = before.stages,
            .src_access = before.access,
            .dst_stages = after.stages,
            .dst_access = after.access,
            .old_layout = before.layout,
            .new_layout = after.layout,
        };
    }

    gpu::BufferBarrier RenderGraph::make_buffer_barrier(
        gpu::Buffer* buffer,
        const RGResourceState& before,
        const RGResourceState& after) const
    {
        return gpu::BufferBarrier{
            .buffer     = buffer,
            .src_stages = before.stages,
            .src_access = before.access,
            .dst_stages = after.stages,
            .dst_access = after.access,
            .size       = gpu::buffer_whole_size,
            .offset     = 0ull,
        };
    }
} // namespace orion
