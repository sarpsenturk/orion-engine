#pragma once

#include "orion/renderapi/handle.hpp"

#include <array>
#include <span>

namespace orion
{
    struct Rect2D {
        std::int32_t x;
        std::int32_t y;
        std::int32_t width;
        std::int32_t height;
    };

    struct RenderAttachment {
        ImageViewHandle render_target;
        std::array<float, 4> clear_color;
    };

    struct CmdBeginRendering {
        std::span<const RenderAttachment> render_targets;
        Rect2D render_area;
    };

    enum class ImageState {
        Unknown,
        RenderTarget,
        Present,
        ShaderResource,
    };

    struct CmdTransitionBarrier {
        ImageHandle image;
        ImageState before;
        ImageState after;
    };

    struct CmdSetPipeline {
        PipelineHandle pipeline;
        PipelineLayoutHandle layout;
    };

    struct Viewport {
        float x;
        float y;
        float width;
        float height;
        float min_depth;
        float max_depth;
    };

    struct CmdSetViewports {
        std::uint32_t start_viewport;
        std::span<const Viewport> viewports;
    };

    struct CmdSetScissors {
        std::uint32_t start_scissor;
        std::span<const Rect2D> scissors;
    };

    struct VertexBufferView {
        BufferHandle buffer;
        std::uint32_t offset;
        std::uint32_t stride;
    };

    struct CmdSetVertexBuffers {
        std::uint32_t start_buffer;
        std::span<const VertexBufferView> vertex_buffers;
    };

    enum class IndexType {
        U16,
        U32,
    };

    struct CmdSetIndexBuffer {
        BufferHandle buffer;
        IndexType index_type;
    };

    struct CmdSetBindGroup {
        std::uint32_t index;
        BindGroupHandle bind_group;
        PipelineLayoutHandle pipeline_layout;
    };

    struct CmdDrawInstanced {
        std::uint32_t vertex_count;
        std::uint32_t instance_count;
        std::uint32_t start_vertex;
        std::uint32_t start_instance;
    };

    struct CmdDrawIndexedInstanced {
        std::uint32_t index_count;
        std::uint32_t instance_count;
        std::uint32_t first_index;
        std::uint32_t first_vertex;
        std::uint32_t first_instance;
    };

    class CommandList
    {
    public:
        CommandList() = default;
        virtual ~CommandList() = default;

        void begin();
        void end();

        void begin_rendering(const CmdBeginRendering& cmd);
        void end_rendering();

        void transition_barrier(const CmdTransitionBarrier& cmd);

        void set_pipeline(const CmdSetPipeline& cmd);
        void set_viewports(const CmdSetViewports& cmd);
        void set_scissors(const CmdSetScissors& cmd);
        void set_vertex_buffers(const CmdSetVertexBuffers& cmd);
        void set_index_buffer(const CmdSetIndexBuffer& cmd);
        void set_bind_group(const CmdSetBindGroup& cmd);

        void draw_instanced(const CmdDrawInstanced& cmd);
        void draw_indexed_instanced(const CmdDrawIndexedInstanced& cmd);

    protected:
        CommandList(const CommandList&) = default;
        CommandList& operator=(const CommandList&) = default;
        CommandList(CommandList&&) = default;
        CommandList& operator=(CommandList&&) = default;

    private:
        virtual void begin_api() = 0;
        virtual void end_api() = 0;

        virtual void begin_rendering_api(const CmdBeginRendering& cmd) = 0;
        virtual void end_rendering_api() = 0;

        virtual void transition_barrier_api(const CmdTransitionBarrier& cmd) = 0;

        virtual void set_pipeline_api(const CmdSetPipeline& cmd) = 0;
        virtual void set_viewports_api(const CmdSetViewports& cmd) = 0;
        virtual void set_scissors_api(const CmdSetScissors& cmd) = 0;
        virtual void set_vertex_buffers_api(const CmdSetVertexBuffers& cmd) = 0;
        virtual void set_index_buffer_api(const CmdSetIndexBuffer& cmd) = 0;
        virtual void set_bind_group_api(const CmdSetBindGroup& cmd) = 0;

        virtual void draw_instanced_api(const CmdDrawInstanced& cmd) = 0;
        virtual void draw_indexed_instanced_api(const CmdDrawIndexedInstanced& cmd) = 0;
    };

    class CommandAllocator
    {
    public:
        CommandAllocator() = default;
        virtual ~CommandAllocator() = default;

        void reset();

    protected:
        CommandAllocator(const CommandAllocator&) = default;
        CommandAllocator& operator=(const CommandAllocator&) = default;
        CommandAllocator(CommandAllocator&&) = default;
        CommandAllocator& operator=(CommandAllocator&&) = default;

    private:
        virtual void reset_api() = 0;
    };

    struct CommandAllocatorDesc {
    };

    struct CommandListDesc {
        const CommandAllocator* command_allocator;
    };
} // namespace orion
