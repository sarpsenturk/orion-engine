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
        explicit CommandAllocator(std::size_t max_size)
            : buffer_(max_size)
            , mbr_(buffer_.data(), buffer_.size())
            , allocator_(&mbr_)
        {
        }

        template<typename Command>
            requires(is_valid_cmd_type<std::remove_cvref_t<Command>>::value)
        Command* allocate()
        {
            return allocator_.new_object<Command>();
        }

    private:
        std::vector<char> buffer_;
        std::pmr::monotonic_buffer_resource mbr_;
        std::pmr::polymorphic_allocator<> allocator_;
    };

    template<typename Key>
    struct CommandPacket {
        Key key;
        const void* data;
    };

    struct CommandBufferDesc {
        CommandQueueType queue_type = {};
        std::size_t buffer_size{};
    };

    template<typename Key>
    class CommandBuffer
    {
    public:
        using key_type = Key;

        CommandBuffer(CommandBufferHandle handle, CommandBufferDesc desc)
            : handle_(handle)
            , desc_(desc)
            , command_allocator_(desc.buffer_size)
        {
        }

        template<typename Command>
        Command* add_command(key_type key)
        {
            auto* command = command_allocator_.allocate<Command>();
            command_packets_.emplace_back(key, command);
            return command;
        }

        [[nodiscard]] auto handle() const noexcept { return handle_; }
        [[nodiscard]] auto queue_type() const noexcept { return desc_.queue_type; }
        [[nodiscard]] auto buffer_size() const noexcept { return desc_.buffer_size; }
        [[nodiscard]] auto& description() const noexcept { return desc_; }

        [[nodiscard]] auto& commands() const noexcept { return command_packets_; }

    private:
        CommandBufferHandle handle_;
        CommandBufferDesc desc_;

        std::vector<CommandPacket<key_type>> command_packets_;
        CommandAllocator command_allocator_;
    };

    using TransferCommandBuffer = CommandBuffer<std::uint8_t>;
    using GraphicsCommandBuffer = CommandBuffer<std::uint32_t>;

    // All available commands
    template<CommandType CommandType, CommandQueueType QueueType>
    struct CmdBase {
        static constexpr auto command_type = CommandType;
        static constexpr auto queue_type = QueueType;
    };

    struct CmdBufferCopy : CmdBase<CommandType::BufferCopy, CommandQueueType::Transfer> {
        GPUBufferHandle src;
        GPUBufferHandle dst;
        std::size_t size;
    };

    struct CmdBeginFrame : CmdBase<CommandType::BeginFrame, CommandQueueType::Graphics> {
        RenderTargetHandle render_target;
        math::Vector4_f clear_color;
    };

    struct CmdEndFrame : CmdBase<CommandType::EndFrame, CommandQueueType::Graphics> {
    };

    struct CmdDraw : CmdBase<CommandType::Draw, CommandQueueType::Graphics> {
        GPUBufferHandle vertex_buffer;
        PipelineHandle graphics_pipeline;
    };
} // namespace orion
