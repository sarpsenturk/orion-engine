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

    void CommandAllocator::reset()
    {
        reset_api();
    }

    std::unique_ptr<CommandList> CommandAllocator::create_command_list()
    {
        return create_command_list_api();
    }
} // namespace orion
