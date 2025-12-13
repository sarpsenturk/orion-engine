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

    void RHICommandList::draw_instanced(const RHICmdDrawInstanced& cmd)
    {
        draw_instanced_api(cmd);
    }
} // namespace orion
