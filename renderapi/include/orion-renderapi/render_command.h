#pragma once

#include "orion-renderapi/image.h"
#include "orion-renderapi/pipeline.h"
#include "orion-renderapi/render_queue.h"
#include "orion-renderapi/types.h"

#include "orion-math/vector/vector4.h"

#include <cstdint>
#include <memory>

namespace orion
{
    struct CmdDraw {
        std::uint32_t vertex_count;
        std::uint32_t instance_count;
        std::uint32_t first_vertex;
        std::uint32_t first_instance;
    };

    struct CmdDrawIndexed {
        std::uint32_t index_count;
        std::uint32_t instance_count;
        std::uint32_t first_index;
        std::int32_t vertex_offset;
        std::uint32_t first_instance;
    };

    struct CmdBindIndexBuffer {
        GPUBufferHandle index_buffer;
        std::size_t offset;
        IndexType index_type;
    };

    struct CmdBindVertexBuffer {
        GPUBufferHandle vertex_buffer;
        std::size_t offset;
    };

    struct CmdBindPipeline {
        PipelineHandle pipeline;
        PipelineBindPoint bind_point;
    };

    struct CmdBindDescriptor {
        PipelineBindPoint bind_point;
        PipelineLayoutHandle pipeline_layout;
        std::uint32_t index;
        DescriptorHandle descriptor;
    };

    struct CmdBeginRenderPass {
        RenderPassHandle render_pass;
        FramebufferHandle framebuffer;
        Rect2D render_area;
        Vector4_f clear_color;
    };

    struct CmdSetViewports {
        std::uint32_t first_viewport = 0;
        std::span<const Viewport> viewports;
    };

    struct CmdSetScissors {
        std::uint32_t first_scissor = 0;
        std::span<const Scissor> scissors;
    };

    struct CmdCopyBuffer {
        GPUBufferHandle src;
        GPUBufferHandle dst;
        std::size_t src_offset;
        std::size_t dst_offset;
        std::size_t size;
    };

    struct CmdCopyBufferToImage {
        GPUBufferHandle src_buffer;
        ImageHandle dst_image;
        ImageLayout dst_layout;
        std::size_t buffer_offset;
        std::size_t image_offset;
        Vector3_u dst_size;
    };

    struct CmdPushConstants {
        PipelineLayoutHandle pipeline_layout;
        ShaderStageFlags shader_stages;
        std::uint32_t offset;
        // TODO: size & ptr interface is bad. look into using span
        std::uint32_t size;
        const void* values;
    };

    struct CmdTransitionBarrier {
        ImageHandle image;
        ImageLayout old_layout;
        ImageLayout new_layout;
    };

    class CommandList
    {
    public:
        CommandList() = default;
        virtual ~CommandList() = default;

        void begin();
        void end();
        void reset();
        void draw(const CmdDraw& cmd_draw);
        void draw_indexed(const CmdDrawIndexed& cmd_draw_indexed);
        void bind_index_buffer(const CmdBindIndexBuffer& cmd_bind_index_buffer);
        void bind_vertex_buffer(const CmdBindVertexBuffer& cmd_bind_vertex_buffer);
        void bind_pipeline(const CmdBindPipeline& cmd_bind_pipeline);
        void bind_descriptor(const CmdBindDescriptor& cmd_bind_descriptor);
        void begin_render_pass(const CmdBeginRenderPass& cmd_begin_render_pass);
        void end_render_pass();
        void set_viewports(const CmdSetViewports& cmd_set_viewports);
        void set_viewports(const Viewport& viewport);
        void set_scissors(const CmdSetScissors& cmd_set_scissors);
        void set_scissors(const Scissor& scissor);
        void copy_buffer(const CmdCopyBuffer& cmd_copy_buffer);
        void copy_buffer_to_image(const CmdCopyBufferToImage& cmd_copy_buffer_to_image);
        void push_constants(const CmdPushConstants& cmd_push_constants);
        void transition_barrier(const CmdTransitionBarrier& cmd_transition_barrier);

    protected:
        CommandList(const CommandList&) = default;
        CommandList(CommandList&&) noexcept = default;
        CommandList& operator=(const CommandList&) = default;
        CommandList& operator=(CommandList&&) noexcept = default;

    private:
        virtual void begin_api() = 0;
        virtual void end_api() = 0;
        virtual void reset_api() = 0;
        virtual void draw_api(const CmdDraw& cmd_draw) = 0;
        virtual void draw_indexed_api(const CmdDrawIndexed& cmd_draw_indexed) = 0;
        virtual void bind_index_buffer_api(const CmdBindIndexBuffer& cmd_bind_index_buffer) = 0;
        virtual void bind_vertex_buffer_api(const CmdBindVertexBuffer& cmd_bind_vertex_buffer) = 0;
        virtual void bind_pipeline_api(const CmdBindPipeline& cmd_bind_pipeline) = 0;
        virtual void bind_descriptor_api(const CmdBindDescriptor& cmd_bind_descriptor) = 0;
        virtual void begin_render_pass_api(const CmdBeginRenderPass& cmd_begin_render_pass) = 0;
        virtual void end_render_pass_api() = 0;
        virtual void set_viewports_api(const CmdSetViewports& cmd_set_viewports) = 0;
        virtual void set_scissors_api(const CmdSetScissors& cmd_set_scissors) = 0;
        virtual void copy_buffer_api(const CmdCopyBuffer& cmd_copy_buffer) = 0;
        virtual void copy_buffer_to_image_api(const CmdCopyBufferToImage& cmd_copy_buffer_to_image) = 0;
        virtual void push_constants_api(const CmdPushConstants& cmd_push_constants) = 0;
        virtual void transition_barrier_api(const CmdTransitionBarrier& cmd_transition_barrier) = 0;
    };

    using CommandListPtr = std::unique_ptr<CommandList>;

    struct CommandAllocatorDesc {
        CommandQueueType queue_type;
        bool reset_command_buffer;
    };

    class CommandAllocator
    {
    public:
        CommandAllocator() = default;
        virtual ~CommandAllocator() = default;

        void reset();
        [[nodiscard]] std::unique_ptr<CommandList> create_command_list();

    private:
        CommandAllocator(const CommandAllocator&) = default;
        CommandAllocator(CommandAllocator&&) noexcept = default;
        CommandAllocator& operator=(const CommandAllocator&) = default;
        CommandAllocator& operator=(CommandAllocator&&) noexcept = default;

        virtual void reset_api() = 0;
        [[nodiscard]] virtual std::unique_ptr<CommandList> create_command_list_api() = 0;
    };

    using CommandAllocatorPtr = std::unique_ptr<CommandAllocator>;
} // namespace orion
