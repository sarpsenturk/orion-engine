#include "orion/renderapi/render_command.h"

#include "orion/assertion.h"

namespace orion
{
    void CommandList::begin()
    {
        begin_api();
    }

    void CommandList::end()
    {
        end_api();
    }

    void CommandAllocator::reset()
    {
        reset_api();
    }

    void CommandList::begin_rendering(const CmdBeginRendering& cmd)
    {
        ORION_ASSERT(cmd.render_area.width != 0);
        ORION_ASSERT(cmd.render_area.height != 0);
        begin_rendering_api(cmd);
    }

    void CommandList::end_rendering()
    {
        end_rendering_api();
    }

    void CommandList::transition_barrier(const CmdTransitionBarrier& cmd)
    {
        transition_barrier_api(cmd);
    }

    void CommandList::set_pipeline(const CmdSetPipeline& cmd)
    {
        set_pipeline_api(cmd);
    }

    void CommandList::set_viewports(const CmdSetViewports& cmd)
    {
        set_viewports_api(cmd);
    }

    void CommandList::set_scissors(const CmdSetScissors& cmd)
    {
        set_scissors_api(cmd);
    }

    void CommandList::set_vertex_buffers(const CmdSetVertexBuffers& cmd)
    {
        set_vertex_buffers_api(cmd);
    }

    void CommandList::set_index_buffer(const CmdSetIndexBuffer& cmd)
    {
        set_index_buffer_api(cmd);
    }

    void CommandList::set_descriptor_set(const CmdSetDescriptorSet& cmd)
    {
        set_descriptor_set_api(cmd);
    }

    void CommandList::draw_instanced(const CmdDrawInstanced& cmd)
    {
        draw_instanced_api(cmd);
    }

    void CommandList::draw_indexed_instanced(const CmdDrawIndexedInstanced& cmd)
    {
        draw_indexed_instanced_api(cmd);
    }
} // namespace orion
