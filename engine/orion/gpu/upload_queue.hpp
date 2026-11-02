#pragma once

#include "gpu.hpp"
#include "bump_allocator.hpp"

#include <array>
#include <vector>

namespace orion::gpu
{
    struct UploadAccess {
        PipelineStage stages = PipelineStage::None;
        Access access        = Access::None;
    };

    struct UploadImageState {
        UploadAccess access = {};
        ImageLayout layout  = ImageLayout::Undefined;
    };

    struct BufferUpload {
        Buffer* dst         = nullptr;
        std::size_t offset  = 0z;
        UploadAccess before = {};
        UploadAccess after  = {};
    };

    struct ImageUpload {
        Image* dst                    = nullptr;
        SubresourceLayers subresource = {};
        Offset2D offset               = {};
        Extent2D extent               = {};
        UploadImageState before       = {};
        UploadImageState after        = {};
    };

    // Single-threaded graphics-queue uploads. Device and destinations are borrowed.
    // Destinations must survive GPU completion; source bytes are copied immediately.
    class UploadQueue
    {
    public:
        explicit UploadQueue(Device* device);
        UploadQueue(const UploadQueue&) = delete;
        UploadQueue& operator=(const UploadQueue&) = delete;
        UploadQueue(UploadQueue&&) = delete;
        UploadQueue& operator=(UploadQueue&&) = delete;
        // Discards pending uploads and waits for submitted work before cleanup.
        ~UploadQueue();

        void upload_buffer(const BufferUpload& upload, ByteSpan data);
        // Tightly packed data for the selected mip/layers, in the image's format.
        // Caller provides valid bounds and states; Undefined discards prior contents.
        void upload_image(const ImageUpload& upload, ByteSpan data);
        [[nodiscard]] std::optional<TimelinePoint> flush();
        // Flush and wait for all submitted uploads.
        void finish();

    private:
        struct Page {
            Buffer* buffer;
            BumpAllocator allocator;
        };
        struct Batch {
            CommandAllocator* allocator = nullptr;
            CommandBuffer* commands     = nullptr;
            std::vector<Page> pages;
            std::optional<TimelinePoint> completion;
        };
        struct StagingSlice {
            Buffer* buffer;
            std::size_t offset;
        };

        Batch& begin_batch();
        StagingSlice stage(Batch& batch, ByteSpan data);
        void wait(Batch& batch);

        Device* m_device;
        std::array<Batch, 3> m_batches;
        std::size_t m_current = 0z;
    };
}
