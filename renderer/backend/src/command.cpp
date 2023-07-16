#include "orion-renderapi/command.h"
#include "orion-renderapi/render_device.h"

#include "orion-utils/assertion.h"

#ifndef ORION_COMMAND_BUFFER_LOG_LEVEL
    #define ORION_COMMAND_BUFFER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    LinearCommandAllocator::LinearCommandAllocator(std::size_t max_size)
        : buffer_(max_size)
        , mbr_(buffer_.data(), buffer_.size())
        , allocator_(&mbr_)
    {
    }

    void* LinearCommandAllocator::allocate(std::size_t bytes, std::size_t alignment)
    {
        return allocator_.allocate_bytes(bytes, alignment);
    }

    void LinearCommandAllocator::reset()
    {
        mbr_.release();
    }

    CommandList::CommandList(RenderDevice* device, CommandBufferHandle handle)
        : device_(device)
        , handle_(handle)
    {
        ORION_EXPECTS(handle_.is_valid());
    }

    void CommandList::begin(const CommandBufferBeginDesc& desc)
    {
        ORION_ASSERT(!is_recording() && "Command buffer already in the recording state");
        ORION_ASSERT(!is_pending() && "Command buffer must leave pending state before recording");
        device_->begin_command_buffer(handle_, desc);
        set_state(CommandBufferState::Recording);
    }

    void CommandList::end()
    {
        ORION_ASSERT(is_recording() && "Command buffer is not in recording state");
        flush();
        device_->end_command_buffer(handle_);
        set_state(CommandBufferState::Executable);
    }

    void CommandList::reset()
    {
        ORION_ASSERT(!is_pending() && "Command buffer must leave pending state before resetting");
        device_->reset_command_buffer(handle_);
        set_state(CommandBufferState::Initial);
    }

    void CommandList::flush()
    {
        device_->compile_commands(handle_, command_packets_);
    }

    void CommandList::set_state(CommandBufferState state) noexcept
    {
        state_ = state;
    }

    void* CommandList::add_command(std::uint64_t key, std::size_t size, std::size_t align, CommandType type)
    {
        void* cmd_ptr = command_allocator_.allocate(size, align);
        command_packets_.push_back({key, type, cmd_ptr});
        return cmd_ptr;
    }

    spdlog::logger* CommandList::logger()
    {
        static const auto logger = create_logger("orion-render-cmd", static_cast<spdlog::level::level_enum>(ORION_COMMAND_BUFFER_LOG_LEVEL));
        return logger.get();
    }
} // namespace orion
