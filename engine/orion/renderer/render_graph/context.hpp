#pragma once

#include "resource.hpp"

#include "orion/renderer/gpu/gpu.hpp"

namespace orion
{
    class RenderPassContext
    {
    public:
        gpu::CommandBuffer* command_buffer() const { return m_command_buffer; }
        [[nodiscard]] gpu::Image* image(RGTexture texture) const;
        [[nodiscard]] gpu::ImageView* image_view(RGTexture texture) const;
        [[nodiscard]] gpu::Buffer* buffer(RGBuffer buffer) const;

    private:
        friend class RenderGraph;
        RenderPassContext(class RenderGraph* graph, gpu::CommandBuffer* command_buffer);

        class RenderGraph* m_graph;
        gpu::CommandBuffer* m_command_buffer;
    };
} // namespace orion
