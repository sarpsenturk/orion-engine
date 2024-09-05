#include "orion/renderapi/render_command.h"

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
} // namespace orion
