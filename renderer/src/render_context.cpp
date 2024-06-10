#include "orion-renderer/render_context.h"

#include "orion-renderer/types.h"

#include "orion-renderapi/render_device.h"

#include <cstring>

// TODO:
//  staging buffer copies should be done via a function which can handle
//  copying of data larger than the staging buffer by dividing copies.
//  Currently attempting to stage any data larger than staging_buffer_size
//  will simply fail.

namespace orion
{
    TransferContext::TransferContext(RenderDevice* device, CommandAllocator* command_allocator, GPUBufferHandle staging_buffer)
        : device_(device)
        , command_allocator_(command_allocator)
        , staging_buffer_(staging_buffer)
    {
    }

    void TransferContext::copy_buffer_staging(const CopyBufferStaging& copy)
    {
        copy_buffer_staging({&copy, 1});
    }

    void TransferContext::copy_buffer_staging(std::span<const CopyBufferStaging> copies)
    {
        auto command_list = command_allocator_->create_command_list();
        command_list->begin();
        const auto src_buffer = staging_buffer_;
        // TODO: Persistent mapping could be a better option here
        void* base = device_->map(staging_buffer_);
        // TODO: Assert size of copies is less than staging_buffer_size
        //  or copy staging_buffer_size bytes at a time
        for (std::size_t offset = 0; const auto& copy : copies) {
            std::memcpy(static_cast<char*>(base) + offset, copy.bytes.data(), copy.bytes.size_bytes());
            command_list->copy_buffer({.src = src_buffer, .dst = copy.dst, .src_offset = offset, .dst_offset = copy.dst_offset, .size = copy.bytes.size_bytes()});
            offset += copy.bytes.size_bytes();
        }
        device_->unmap(staging_buffer_);
        command_list->end();
        device_->submit_immediate({.queue_type = CommandQueueType::Graphics, .command_lists = {{command_list.get()}}});
    }

    void TransferContext::copy_image_staging(const CopyImageStaging& copy)
    {
        auto cmd_list = command_allocator_->create_command_list();
        cmd_list->begin();
        cmd_list->transition_barrier({.image = copy.dst, .old_layout = copy.dst_initial_layout, .new_layout = ImageLayout::TransferDst});
        const auto staging = staging_buffer_;
        // Upload image data to staging buffer
        {
            void* dst = device_->map(staging);
            std::memcpy(dst, copy.bytes.data(), copy.bytes.size_bytes());
            device_->unmap(staging);
        }
        cmd_list->copy_buffer_to_image({
            .src_buffer = staging,
            .dst_image = copy.dst,
            .dst_layout = ImageLayout::TransferDst,
            .buffer_offset = 0,
            .image_offset = 0,
            .dst_size = copy.dst_size,
        });
        cmd_list->transition_barrier({.image = copy.dst, .old_layout = ImageLayout::TransferDst, .new_layout = copy.dst_final_layout});
        cmd_list->end();
        device_->submit_immediate({.queue_type = CommandQueueType::Graphics, .command_lists = {{cmd_list.get()}}});
    }

    void TransferContext::memcpy(ImageHandle image, std::span<const std::byte> bytes, std::size_t offset)
    {
        void* dst = device_->map(image);
        dst = static_cast<char*>(dst) + offset;
        std::memcpy(dst, bytes.data(), bytes.size_bytes());
        device_->unmap(image);
    }
} // namespace orion
