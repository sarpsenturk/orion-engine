#include "orion-renderapi/command.h"
#include "orion-renderapi/render_device.h"

#include "orion-utils/assertion.h"

namespace orion
{
    CommandList::CommandList(std::size_t max_size)
        : command_allocator_(max_size)
    {
    }

    void CommandList::begin()
    {
        ORION_ASSERT(!is_recording() && "Command buffer already in the recording state");
        ORION_ASSERT(!is_pending() && "Command buffer must leave pending state before recording");
        set_state(CommandBufferState::Recording);
    }

    void CommandList::end()
    {
        ORION_ASSERT(is_recording() && "Command buffer is not in recording state");
        set_state(CommandBufferState::Executable);
    }

    void CommandList::reset()
    {
        ORION_ASSERT(!is_pending() && "Command buffer must leave pending state before resetting");
        command_packets_.clear();
        command_allocator_.reset();
        set_state(CommandBufferState::Initial);
    }

    void CommandList::set_state(CommandBufferState state) noexcept
    {
        state_ = state;
    }

    void* CommandList::add_command(std::uint64_t key, std::size_t size, std::size_t align, CommandType type)
    {
        ORION_ASSERT(is_recording() && "Command buffer must be in recording state before adding commands");
        auto [cmd_ptr, cmd_size] = command_allocator_.allocate(size, align);
        command_packets_.push_back({key, type, cmd_ptr});
        return cmd_ptr;
    }
} // namespace orion
