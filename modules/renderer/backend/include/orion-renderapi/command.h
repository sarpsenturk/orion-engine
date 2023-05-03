#pragma once

#include "handles.h"
#include "types.h"

#include <memory_resource> // std::pmr:
#include <orion-math/vector/vector4.h>
#include <type_traits>
#include <vector>

namespace orion
{
    // This is the implementation of std::is_implicit_lifetime as seen at: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2674r0.pdf
    // We require commands to be implicit lifetime types, so we can simply allocate memory for them and move on
    template<typename T>
    struct is_valid_cmd_type
        : std::disjunction<
              std::is_scalar<T>,
              std::is_array<T>,
              std::is_aggregate<T>,
              std::conjunction<
                  std::is_trivially_destructible<T>,
                  std::disjunction<
                      std::is_trivially_default_constructible<T>,
                      std::is_trivially_copy_constructible<T>,
                      std::is_trivially_move_constructible<T>>>> {
    };

    class CommandAllocator
    {
    public:
        CommandAllocator() = default;
        virtual ~CommandAllocator() = default;

        virtual void* allocate(std::size_t bytes, std::size_t alignment) = 0;
        virtual void reset() = 0;

    protected:
        CommandAllocator(const CommandAllocator&) = default;
        CommandAllocator(CommandAllocator&&) noexcept = default;
        CommandAllocator& operator=(const CommandAllocator&) = default;
        CommandAllocator& operator=(CommandAllocator&&) = default;
    };

    class LinearCommandAllocator final : public CommandAllocator
    {
    public:
        LinearCommandAllocator() = default;
        explicit LinearCommandAllocator(std::size_t max_size);

        void* allocate(std::size_t bytes, std::size_t alignment) override;
        void reset() override;

    private:
        std::vector<char> buffer_;
        std::pmr::monotonic_buffer_resource mbr_;
        std::pmr::polymorphic_allocator<> allocator_;
    };

    struct CommandPacket {
        std::uint64_t key;
        CommandType command_type;
        const void* data;
    };

    struct CommandBufferDesc {
        CommandQueueType queue_type = {};
    };

    class CommandBuffer
    {
    public:
        CommandBuffer() = default;
        CommandBuffer(CommandBufferHandle handle, CommandBufferDesc desc, std::unique_ptr<CommandAllocator> command_allocator);

        template<typename Command>
            requires(is_valid_cmd_type<Command>::value)
        Command* add_command(std::uint64_t key)
        {
            auto* command = static_cast<Command*>(command_allocator_->allocate(sizeof(Command), alignof(Command)));
            command_packets_.emplace_back(key, Command::command_type, command);
            return command;
        }

        void reset();

        [[nodiscard]] auto handle() const noexcept { return handle_; }
        [[nodiscard]] auto queue_type() const noexcept { return desc_.queue_type; }
        [[nodiscard]] auto& description() const noexcept { return desc_; }

        [[nodiscard]] auto& commands() const noexcept { return command_packets_; }

    private:
        CommandBufferHandle handle_;
        CommandBufferDesc desc_;
        std::unique_ptr<CommandAllocator> command_allocator_;

        std::vector<CommandPacket> command_packets_;
    };

    // All available commands
    template<CommandType CommandType, CommandQueueType QueueType>
    struct CmdBase {
        static constexpr auto command_type = CommandType;
        static constexpr auto queue_type = QueueType;
    };

    struct CmdBufferCopy : CmdBase<CommandType::BufferCopy, CommandQueueType::Transfer> {
        GPUBufferHandle dst;
        std::size_t dst_offset;
        GPUBufferHandle src;
        std::size_t src_offset;
        std::size_t size;
    };

    struct CmdBeginFrame : CmdBase<CommandType::BeginFrame, CommandQueueType::Graphics> {
        RenderTargetHandle render_target;
        math::Vector2_u render_area;
        math::Vector4_f clear_color;
    };

    struct CmdEndFrame : CmdBase<CommandType::EndFrame, CommandQueueType::Graphics> {
    };

    struct CmdDraw : CmdBase<CommandType::Draw, CommandQueueType::Graphics> {
        GPUBufferHandle vertex_buffer;
        PipelineHandle graphics_pipeline;
        Viewport viewport;
        std::uint32_t vertex_count;
        std::uint32_t first_vertex;
    };

    struct CmdDrawIndexed : CmdBase<CommandType::DrawIndexed, CommandQueueType::Graphics> {
        GPUBufferHandle vertex_buffer;
        GPUBufferHandle index_buffer;
        PipelineHandle graphics_pipeline;
        Viewport viewport;
        std::uint32_t index_count;
    };
} // namespace orion
