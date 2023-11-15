#pragma once

#include "handles.h"
#include "types.h"

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

    class CommandList
    {
    public:
        CommandList() = default;
        virtual ~CommandList() = default;

        void begin();
        void end();
        void draw(const CmdDraw& cmd_draw);
        void draw_indexed(const CmdDrawIndexed& cmd_draw_indexed);
        void bind_index_buffer(const CmdBindIndexBuffer& cmd_bind_index_buffer);
        void bind_vertex_buffer(const CmdBindVertexBuffer& cmd_bind_vertex_buffer);

    protected:
        CommandList(const CommandList&) = default;
        CommandList(CommandList&&) noexcept = default;
        CommandList& operator=(const CommandList&) = default;
        CommandList& operator=(CommandList&&) noexcept = default;

    private:
        virtual void begin_api() = 0;
        virtual void end_api() = 0;
        virtual void draw_api(const CmdDraw& cmd_draw) = 0;
        virtual void draw_indexed_api(const CmdDrawIndexed& cmd_draw_indexed) = 0;
        virtual void bind_index_buffer_api(const CmdBindIndexBuffer& cmd_bind_index_buffer) = 0;
        virtual void bind_vertex_buffer_api(const CmdBindVertexBuffer& cmd_bind_vertex_buffer) = 0;
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
} // namespace orion
