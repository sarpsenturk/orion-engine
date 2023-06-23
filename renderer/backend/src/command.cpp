#include "orion-renderapi/command.h"

namespace orion
{
    CommandBuffer::CommandBuffer(CommandBufferHandle handle, std::unique_ptr<CommandAllocator> command_allocator)
        : handle_(handle)
        , command_allocator_(std::move(command_allocator))
    {
    }

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

    void CommandBuffer::reset()
    {
        command_packets_.clear();
        command_allocator_->reset();
    }
} // namespace orion
