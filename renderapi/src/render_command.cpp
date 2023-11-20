#include "orion-renderapi/render_command.h"

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

    void CommandList::draw(const CmdDraw& cmd_draw)
    {
        draw_api(cmd_draw);
    }

    void CommandList::draw_indexed(const CmdDrawIndexed& cmd_draw_indexed)
    {
        draw_indexed_api(cmd_draw_indexed);
    }

    void CommandList::bind_index_buffer(const CmdBindIndexBuffer& cmd_bind_index_buffer)
    {
        bind_index_buffer_api(cmd_bind_index_buffer);
    }

    void CommandList::bind_vertex_buffer(const CmdBindVertexBuffer& cmd_bind_vertex_buffer)
    {
        bind_vertex_buffer_api(cmd_bind_vertex_buffer);
    }

    void CommandList::bind_pipeline(const CmdBindPipeline& cmd_bind_pipeline)
    {
        bind_pipeline_api(cmd_bind_pipeline);
    }

    void CommandList::bind_descriptor(const CmdBindDescriptor& cmd_bind_descriptor)
    {
        bind_descriptor_api(cmd_bind_descriptor);
    }

    void CommandList::begin_render_pass(const CmdBeginRenderPass& cmd_begin_render_pass)
    {
        begin_render_pass_api(cmd_begin_render_pass);
    }

    void CommandList::end_render_pass()
    {
        end_render_pass_api();
    }

    void CommandAllocator::reset()
    {
        reset_api();
    }

    std::unique_ptr<CommandList> CommandAllocator::create_command_list()
    {
        return create_command_list_api();
    }
} // namespace orion
