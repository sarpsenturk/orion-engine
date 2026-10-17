#include "builder.hpp"
#include "graph.hpp"

#include "orion/core/assert.hpp"

namespace orion
{
    RenderPassBuilder::RenderPassBuilder(class RenderGraph* graph, std::uint32_t pass_index)
        : m_graph(graph)
        , m_pass_index(pass_index)
    {
    }

    RGTexture RenderPassBuilder::add_color_attachment(
        RGTexture texture,
        gpu::LoadOp load_op,
        gpu::StoreOp store_op,
        gpu::ClearColorValue clear_color)
    {
        auto& resource  = m_graph->m_resources[texture.index];
        auto& pass_decl = m_graph->m_pass_decls[m_pass_index];
        ORION_ASSERT(pass_decl.color_attachments.size() < gpu::max_color_attachments && "Maximum number of color attachments exceeded");

        // Ensure the texture is valid and the version matches the latest version of the resource
        ORION_ASSERT(texture.version == resource.latest_version_number());
        const auto input_defined = resource.latest_version().contents_defined;

        auto access = gpu::Access::ColorAttachmentWrite;
        // If LoadOp::Load is used, the pass is a reader of the current version of the resource
        if (load_op == gpu::LoadOp::Load) {
            // Ensure there is a previous version to read from
            ORION_ASSERT(input_defined);
            // Add the current pass as a reader of the previous version
            resource.versions[texture.version].reader_passes.push_back(m_pass_index);
            // Add the read access for this pass
            access |= gpu::Access::ColorAttachmentRead;
        }

        // Attachments are assumed as writes and generate a new version of the resource
        auto& new_version       = resource.add_version();
        new_version.writer_pass = m_pass_index;
        new_version.contents_defined =
            store_op == gpu::StoreOp::Store &&
            (load_op == gpu::LoadOp::Clear ||
             (load_op == gpu::LoadOp::Load && input_defined));

        // Add the resource access for this pass
        pass_decl.resource_accesses.push_back(RenderGraph::ResourceAccess{
            texture.index,
            RGResourceState{
                .stages = gpu::PipelineStage::ColorAttachmentOutput,
                .access = access,
                .layout = gpu::ImageLayout::ColorAttachment,
            },
        });
        // Add the color attachment reference for this pass
        pass_decl.color_attachments.emplace_back(texture.index, load_op, store_op, gpu::ClearValue{clear_color});
        return RGTexture{texture.index, texture.version + 1};
    }
} // namespace orion
