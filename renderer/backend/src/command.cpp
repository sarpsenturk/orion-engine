#include "orion-renderapi/command.h"
#include "orion-renderapi/render_device.h"

#include "orion-utils/assertion.h"

namespace orion
{
    CommandList::CommandList(RenderDevice* device, CommandBufferHandle command_buffer, std::size_t size)
        : device_(device)
        , command_buffer_(command_buffer)
        , command_allocator_(size)
    {
        ORION_EXPECTS(command_buffer_.is_valid());
    }

    void CommandList::begin(const CommandBufferBeginDesc& desc)
    {
        ORION_ASSERT(!is_recording() && "Command buffer already in the recording state");
        ORION_ASSERT(!is_pending() && "Command buffer must leave pending state before recording");
        device_->begin_command_buffer(command_buffer_, desc);
        set_state(CommandBufferState::Recording);
    }

    void CommandList::end()
    {
        ORION_ASSERT(is_recording() && "Command buffer is not in recording state");
        flush();
        clear();
        device_->end_command_buffer(command_buffer_);
        set_state(CommandBufferState::Executable);
    }

    void CommandList::reset()
    {
        ORION_ASSERT(!is_pending() && "Command buffer must leave pending state before resetting");
        clear();
        device_->reset_command_buffer(command_buffer_);
        set_state(CommandBufferState::Initial);
    }

    void CommandList::flush()
    {
        device_->compile_commands(command_buffer_, command_packets_);
    }

    void CommandList::clear()
    {
        command_packets_.clear();
        command_allocator_.reset();
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
