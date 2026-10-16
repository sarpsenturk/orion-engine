#include "context.hpp"
#include "graph.hpp"

namespace orion
{
    RenderPassContext::RenderPassContext(class RenderGraph* graph, gpu::CommandBuffer* command_buffer)
        : m_graph(graph)
        , m_command_buffer(command_buffer)
    {
    }

    gpu::Image* RenderPassContext::image(RGTexture texture) const
    {
        return std::get<RenderGraph::GpuTexture>(m_graph->m_gpu_resources[texture.index]).image;
    }

    gpu::ImageView* RenderPassContext::image_view(RGTexture texture) const
    {
        return std::get<RenderGraph::GpuTexture>(m_graph->m_gpu_resources[texture.index]).view;
    }

    gpu::Buffer* RenderPassContext::buffer(RGBuffer buffer) const
    {
        return std::get<RenderGraph::GpuBuffer>(m_graph->m_gpu_resources[buffer.index]).buffer;
    }
} // namespace orion
