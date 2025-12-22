#include "orion/rhi/command.hpp"

namespace orion
{
    bool RHICommandAllocator::reset()
    {
        return reset_api();
    }

    bool RHICommandList::reset()
    {
        return reset_api();
    }

    void RHICommandList::begin()
    {
        return begin_api();
    }

    void RHICommandList::end()
    {
        return end_api();
    }

    void RHICommandList::pipeline_barrier(const RHICmdPipelineBarrier& cmd)
    {
        pipeline_barrier_api(cmd);
    }

    void RHICommandList::begin_rendering(const RHICmdBeginRendering& cmd)
    {
        begin_rendering_api(cmd);
    }

    void RHICommandList::end_rendering()
    {
        end_rendering_api();
    }

    void RHICommandList::set_graphics_pipeline_state(RHIPipeline pipeline)
    {
        set_graphics_pipeline_state_api(pipeline);
    }

    void RHICommandList::set_viewports(const RHICmdSetViewports& cmd)
    {
        set_viewports_api(cmd);
    }

    void RHICommandList::set_scissors(const RHICmdSetScissors& cmd)
    {
        set_scissors_api(cmd);
    }

    void RHICommandList::set_vertex_buffers(const RHICmdSetVertexBuffers& cmd)
    {
        set_vertex_buffers_api(cmd);
    }

    void RHICommandList::set_index_buffer(const RHICmdSetIndexBuffer& cmd)
    {
        set_index_buffer_api(cmd);
    }

    void RHICommandList::draw_instanced(const RHICmdDrawInstanced& cmd)
    {
        draw_instanced_api(cmd);
    }

    void RHICommandList::draw_indexed_instanced(const RHICmdDrawIndexedInstanced& cmd)
    {
        draw_indexed_instanced_api(cmd);
    }

    void RHICommandQueue::wait(RHIFence fence, std::uint64_t value)
    {
        wait_api(fence, value);
    }

    void RHICommandQueue::signal(RHIFence fence, std::uint64_t value)
    {
        signal_api(fence, value);
    }

    void RHICommandQueue::submit(std::span<const RHICommandList* const> command_lists)
    {
        submit_api(command_lists);
    }
} // namespace orion
