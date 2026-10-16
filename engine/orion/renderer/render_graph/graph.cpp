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

        // Walk through the passes, keeping track of resource states,
        // and insert barriers as needed
        for (std::uint32_t pass_index = 0u; pass_index < m_passes.size(); ++pass_index) {
            auto& pass = m_passes[pass_index];
            ORION_ASSERT(!pass.color_attachments.empty() && "Passes without color attachments are not supported");
            for (const auto& access : pass.resource_accesses) {
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
                        pass.image_barriers.push_back(make_image_barrier(texture->image, state, access.state));
                    } else {
                        auto& buffer = std::get<GpuBuffer>(m_gpu_resources[access.resource_index]);
                        pass.buffer_barriers.push_back(make_buffer_barrier(buffer.buffer, state, access.state));
                    }
                }

                // Update the resource state to the current access state
                state = access.state;
            }
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
        for (const auto& pass : m_passes) {
            // Insert barriers for the pass
            gpu::barrier(command_buffer, gpu::Barrier{pass.image_barriers, pass.buffer_barriers});

            // Begin the render pass
            std::array<gpu::RenderPassAttachment, gpu::max_color_attachments> color_attachments;
            std::ranges::transform(pass.color_attachments, color_attachments.begin(), [this](const AttachmentRef& attachment_ref) {
                const auto& texture = std::get<GpuTexture>(m_gpu_resources[attachment_ref.resource_index]);
                return gpu::RenderPassAttachment{
                    .image_view  = texture.view,
                    .load_op     = attachment_ref.load_op,
                    .store_op    = attachment_ref.store_op,
                    .clear_value = attachment_ref.clear_value,
                };
            });
            gpu::begin_render_pass(
                command_buffer,
                gpu::RenderPass{
                    .color_attachments = std::span{color_attachments.begin(), pass.color_attachments.size()},
                });

            // Execute the pass callback
            auto context = RenderPassContext(this, command_buffer);
            pass.execute(context);

            // End the render pass
            gpu::end_render_pass(command_buffer);
        }

        // Insert final layout transition barriers
        gpu::barrier(command_buffer, gpu::Barrier{m_final_image_barriers, m_final_buffer_barriers});
    }

    void RenderGraph::reset()
    {
        // Clear all internal data structures to prepare for a new frame
        m_passes.clear();
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
