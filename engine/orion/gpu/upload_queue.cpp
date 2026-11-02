#include "upload_queue.hpp"

#include "orion/core/assert.hpp"
#include "orion/utils/scope_guard.hpp"

#include <algorithm>
#include <cstring>
#include <limits>

namespace orion::gpu
{
    UploadQueue::UploadQueue(Device* device)
        : m_device(device)
    {
    }

    UploadQueue::~UploadQueue()
    {
        for (auto& batch : m_batches) {
            wait(batch);
            if (batch.allocator) { destroy_command_allocator(m_device, batch.allocator); }
            for (auto& page : batch.pages) { destroy_buffer(m_device, page.buffer); }
        }
    }

    void UploadQueue::wait(Batch& batch)
    {
        if (batch.completion) {
            wait_timeline(m_device, *batch.completion);
            batch.completion.reset();
        }
    }

    UploadQueue::Batch& UploadQueue::begin_batch()
    {
        auto& batch = m_batches[m_current];
        if (!batch.commands) {
            wait(batch);
            if (!batch.allocator) {
                batch.allocator = create_command_allocator(m_device, Queue::Graphics);
            } else {
                reset_command_allocator(m_device, batch.allocator);
            }
            for (auto& page : batch.pages) { page.allocator.reset(); }
            batch.commands = begin_command_buffer(m_device, batch.allocator);
        }
        return batch;
    }

    UploadQueue::StagingSlice UploadQueue::stage(Batch& batch, ByteSpan data)
    {
        // All currently supported image formats have four-byte texels.
        constexpr auto alignment = 4z;
        constexpr std::size_t page_size = 4z * 1024z * 1024z;
        ORION_ASSERT(data.data != nullptr);
        ORION_ASSERT(!data.empty());
        ORION_ASSERT(data.size <= std::numeric_limits<std::size_t>::max() - alignment);
        for (auto& page : batch.pages) {
            const auto offset = page.allocator.allocate(data.size, alignment);
            if (offset != std::numeric_limits<std::size_t>::max()) {
                std::memcpy(static_cast<std::byte*>(buffer_host_address(page.buffer)) + offset, data.data, data.size);
                return {page.buffer, offset};
            }
        }
        const auto size = std::max(page_size, data.size);
        auto* buffer = create_buffer(m_device, {.size = size, .memory = Memory::CpuVisible});
        const auto guard = ScopeFail{[&] { destroy_buffer(m_device, buffer); }};
        batch.pages.push_back({buffer, BumpAllocator{size}});
        const auto offset = batch.pages.back().allocator.allocate(data.size, alignment);
        std::memcpy(buffer_host_address(buffer), data.data, data.size);
        return {buffer, offset};
    }

    void UploadQueue::upload_buffer(const BufferUpload& upload, ByteSpan data)
    {
        ORION_ASSERT(upload.dst != nullptr);
        auto& batch = begin_batch();
        const auto source = stage(batch, data);
        auto transition = [&](UploadAccess before, UploadAccess after) {
            const auto b = BufferBarrier{
                .buffer = upload.dst,
                .src_stages = before.stages, .src_access = before.access,
                .dst_stages = after.stages, .dst_access = after.access,
                .offset = upload.offset, .size = data.size,
            };
            barrier(batch.commands, {.buffer_barriers = {&b, 1}});
        };
        const auto transfer = UploadAccess{PipelineStage::Transfer, Access::TransferWrite};
        transition(upload.before, transfer);
        const auto region = BufferCopy{source.offset, upload.offset, data.size};
        copy_buffer(batch.commands, source.buffer, upload.dst, {&region, 1});
        transition(transfer, upload.after);
    }

    void UploadQueue::upload_image(const ImageUpload& upload, ByteSpan data)
    {
        ORION_ASSERT(upload.dst != nullptr);
        ORION_ASSERT(upload.extent.width != 0u);
        ORION_ASSERT(upload.extent.height != 0u);
        ORION_ASSERT(upload.subresource.layer_count != 0u);
        ORION_ASSERT(upload.after.layout != ImageLayout::Undefined);
#if ORION_ENABLE_ASSERTIONS
        // Current formats are all four bytes per texel. Check without multiplying
        // untrusted dimensions, which could overflow.
        auto remaining = data.size;
        for (const auto dimension : {upload.extent.width, upload.extent.height, upload.subresource.layer_count, 4u}) {
            ORION_ASSERT(remaining % dimension == 0 && "Image upload size mismatch");
            remaining /= dimension;
        }
        ORION_ASSERT(remaining == 1 && "Image upload size mismatch");
#endif
        auto& batch = begin_batch();
        const auto source = stage(batch, data);
        auto transition = [&](UploadImageState before, UploadImageState after) {
            const auto b = ImageBarrier{
                .image = upload.dst,
                .src_stages = before.access.stages, .src_access = before.access.access,
                .dst_stages = after.access.stages, .dst_access = after.access.access,
                .old_layout = before.layout, .new_layout = after.layout,
                .subresource_range = {upload.subresource.mip_level, 1u,
                                      upload.subresource.base_array_layer, upload.subresource.layer_count},
            };
            barrier(batch.commands, {.image_barriers = {&b, 1}});
        };
        const auto transfer = UploadImageState{{PipelineStage::Transfer, Access::TransferWrite}, ImageLayout::TransferDst};
        transition(upload.before, transfer);
        const auto region = BufferImageCopy{
            .buffer_offset = source.offset,
            .image_subresource = upload.subresource,
            .image_offset = upload.offset,
            .image_extent = upload.extent,
        };
        copy_buffer_to_image(batch.commands, source.buffer, upload.dst, {&region, 1});
        transition(transfer, upload.after);
    }

    std::optional<TimelinePoint> UploadQueue::flush()
    {
        auto& batch = m_batches[m_current];
        if (!batch.commands) { return std::nullopt; }
        const CommandBuffer* commands = batch.commands;
        const auto point = submit(m_device, Queue::Graphics, {.command_buffers = {&commands, 1}});
        batch.completion = point;
        batch.commands = nullptr;
        m_current = (m_current + 1z) % m_batches.size();
        return point;
    }

    void UploadQueue::finish()
    {
        (void)flush();
        for (auto& batch : m_batches) { wait(batch); }
    }
}
