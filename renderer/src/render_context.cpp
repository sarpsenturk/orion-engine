#include "orion-renderer/render_context.h"

#include "orion-renderapi/render_command.h"

#include "orion-utils/assertion.h"

namespace orion
{
    RenderContext::RenderContext(CommandList* command_list, frame_index_t frame_index)
        : command_list_(command_list)
        , frame_index_(frame_index)
    {
        ORION_EXPECTS(command_list != nullptr);
        ORION_EXPECTS(frame_index != -1);
    }
} // namespace orion
