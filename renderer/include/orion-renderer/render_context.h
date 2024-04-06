#pragma once

#include "orion-renderer/config.h"

namespace orion
{
    // Forward declare
    class RenderDevice;
    class CommandList;

    class RenderContext
    {
    public:
        RenderContext(CommandList* command_list, frame_index_t frame_index);

        [[nodiscard]] CommandList* command_list() const noexcept { return command_list_; }
        [[nodiscard]] frame_index_t frame_index() const noexcept { return frame_index_; }

    private:
        CommandList* command_list_;
        frame_index_t frame_index_;
    };
} // namespace orion
