#pragma once

#include "orion/rhi/format.hpp"
#include "orion/rhi/handle.hpp"
#include "orion/rhi/image.hpp"

#include <array>
#include <span>

namespace orion
{
    enum class RHICommandQueueType {
        Graphics,
    };

    struct RHICommandAllocatorDesc {
        RHICommandQueueType type;
    };

    class RHICommandAllocator
    {
    public:
        RHICommandAllocator() = default;
        virtual ~RHICommandAllocator() = default;

        bool reset();

    protected:
        RHICommandAllocator(const RHICommandAllocator&) = default;
        RHICommandAllocator& operator=(const RHICommandAllocator&) = default;
        RHICommandAllocator(RHICommandAllocator&&) = default;
        RHICommandAllocator& operator=(RHICommandAllocator&&) = default;

    private:
        virtual bool reset_api() = 0;
    };

    struct RHICommandListDesc {
        RHICommandAllocator* command_allocator;
    };

    // Command structs

    struct RHITransitionBarrier {
        RHIImage image;
        RHIImageLayout old_layout;
        RHIImageLayout new_layout;
    };

    struct RHICmdPipelineBarrier {
        std::span<const RHITransitionBarrier> transition_barriers;
    };

    struct RHICmdBeginRendering {
        std::uint32_t render_width;
        std::uint32_t render_height;
        std::span<const RHIImageView> rtvs;
        RHIImageView dsv;
        std::array<float, 4> rtv_clear;
        float depth_clear;
    };

    struct RHIViewport {
        float x;
        float y;
        float width;
        float height;
        float min_depth;
        float max_depth;
    };

    struct RHICmdSetViewports {
        std::uint32_t first_viewport;
        std::span<const RHIViewport> viewports;
    };

    struct RHIRect {
        std::int32_t left;
        std::int32_t top;
        std::int32_t right;
        std::int32_t bottom;
    };

    struct RHICmdSetScissors {
        std::span<const RHIRect> scissors;
    };

    struct RHICmdSetVertexBuffers {
        std::uint32_t first_slot;
        std::span<const RHIBuffer> buffers;
        std::span<const std::size_t> offsets;
    };

    struct RHICmdSetIndexBuffer {
        RHIBuffer buffer;
        std::size_t offset;
        RHIFormat format;
    };

    struct RHICmdDrawInstanced {
        std::uint32_t vertex_count;
        std::uint32_t instance_count;
        std::uint32_t first_vertex;
        std::uint32_t first_instance;
    };

    struct RHICmdDrawIndexedInstanced {
        std::uint32_t index_count;
        std::uint32_t instance_count;
        std::uint32_t first_index;
        std::int32_t vertex_offset;
        std::uint32_t first_instance;
    };

    class RHICommandList
    {
    public:
        RHICommandList() = default;
        virtual ~RHICommandList() = default;

        bool reset();

        void begin();
        void end();

        void pipeline_barrier(const RHICmdPipelineBarrier& cmd);

        void begin_rendering(const RHICmdBeginRendering& cmd);
        void end_rendering();

        void set_graphics_pipeline_state(RHIPipeline pipeline);
        void set_viewports(const RHICmdSetViewports& cmd);
        void set_scissors(const RHICmdSetScissors& cmd);
        void set_vertex_buffers(const RHICmdSetVertexBuffers& cmd);
        void set_index_buffer(const RHICmdSetIndexBuffer& cmd);

        void draw_instanced(const RHICmdDrawInstanced& cmd);
        void draw_indexed_instanced(const RHICmdDrawIndexedInstanced& cmd);

    protected:
        RHICommandList(const RHICommandList&) = default;
        RHICommandList& operator=(const RHICommandList&) = default;
        RHICommandList(RHICommandList&&) = default;
        RHICommandList& operator=(RHICommandList&&) = default;

    private:
        virtual bool reset_api() = 0;

        virtual void begin_api() = 0;
        virtual void end_api() = 0;

        virtual void pipeline_barrier_api(const RHICmdPipelineBarrier& cmd) = 0;

        virtual void begin_rendering_api(const RHICmdBeginRendering& cmd) = 0;
        virtual void end_rendering_api() = 0;

        virtual void set_graphics_pipeline_state_api(RHIPipeline pipeline) = 0;
        virtual void set_viewports_api(const RHICmdSetViewports& cmd) = 0;
        virtual void set_scissors_api(const RHICmdSetScissors& cmd) = 0;
        virtual void set_vertex_buffers_api(const RHICmdSetVertexBuffers& cmd) = 0;
        virtual void set_index_buffer_api(const RHICmdSetIndexBuffer& cmd) = 0;

        virtual void draw_instanced_api(const RHICmdDrawInstanced& cmd) = 0;
        virtual void draw_indexed_instanced_api(const RHICmdDrawIndexedInstanced& cmd) = 0;
    };

    struct RHICommandQueueDesc {
        RHICommandQueueType type;
    };

    class RHICommandQueue
    {
    public:
        RHICommandQueue() = default;
        virtual ~RHICommandQueue() = default;

        void wait(RHIFence fence, std::uint64_t value);
        void signal(RHIFence fence, std::uint64_t value);

        void submit(std::span<const RHICommandList* const> command_lists);

    protected:
        RHICommandQueue(const RHICommandQueue&) = default;
        RHICommandQueue& operator=(const RHICommandQueue&) = default;
        RHICommandQueue(RHICommandQueue&&) = default;
        RHICommandQueue& operator=(RHICommandQueue&&) = default;

        virtual void wait_api(RHIFence fence, std::uint64_t value) = 0;
        virtual void signal_api(RHIFence fence, std::uint64_t value) = 0;

        virtual void submit_api(std::span<const RHICommandList* const> command_lists) = 0;
    };
} // namespace orion
