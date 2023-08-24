#pragma once

#include "handles.h"
#include "types.h"

#include "orion-utils/allocators/linear_allocator.h"

#include "orion-math/vector/vector2.h"
#include "orion-math/vector/vector4.h"

#include <spdlog/logger.h>

#include <limits>
#include <type_traits>
#include <vector>

namespace orion
{
    // This is the implementation of std::is_implicit_lifetime as seen at: https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2674r0.pdf
    // We require commands to be implicit lifetime types, so we can simply allocate memory for them and move on
    template<typename T>
    struct IsValidCmdType
        : std::disjunction<
              std::is_scalar<T>,
              std::is_array<T>,
              std::conjunction<std::is_aggregate<T>,
                               std::is_trivially_destructible<T>>,
              std::conjunction<
                  std::is_trivially_destructible<T>,
                  std::disjunction<
                      std::is_trivially_default_constructible<T>,
                      std::is_trivially_copy_constructible<T>,
                      std::is_trivially_move_constructible<T>>>> {
    };

    using command_packet_key_t = std::uint64_t;
    inline constexpr auto command_key_begin = std::numeric_limits<command_packet_key_t>::min();
    inline constexpr auto command_key_end = std::numeric_limits<command_packet_key_t>::max();

    struct CommandPacket {
        command_packet_key_t key;
        CommandType type;
        const void* data;

        friend constexpr bool operator<(const CommandPacket& lhs, const CommandPacket& rhs) noexcept { return lhs.key < rhs.key; }
    };

    // All available commands
    template<CommandType Type, CommandQueueType QueueType>
    struct CmdBase {
        static constexpr auto type = Type;
        static constexpr auto queue_type = QueueType;
    };

#define DEFINE_COMMAND(name, queue_type) \
    struct Cmd##name : CmdBase<CommandType::name, CommandQueueType::queue_type>

    enum class CommandBufferState {
        Initial,
        Recording,
        Executable,
        Pending,
        Invalid
    };

    class RenderDevice;

    class CommandList
    {
    public:
        explicit CommandList(std::size_t max_size);

        template<typename Command>
            requires(IsValidCmdType<Command>::value)
        auto* add_command(command_packet_key_t key)
        {
            return static_cast<Command*>(add_command(key, sizeof(Command), alignof(Command), Command::type));
        }

        [[nodiscard]] auto& commands() const { return command_packets_; }

        [[nodiscard]] auto state() const noexcept { return state_; }
        [[nodiscard]] bool is_recording() const noexcept { return state_ == CommandBufferState::Recording; }
        [[nodiscard]] bool is_executable() const noexcept { return state_ == CommandBufferState::Executable; }
        [[nodiscard]] bool is_pending() const noexcept { return state_ == CommandBufferState::Pending; }
        [[nodiscard]] bool is_invalid() const noexcept { return state_ == CommandBufferState::Invalid; }

        void begin();
        void end();
        void reset();
        void sort();

    private:
        void set_state(CommandBufferState state) noexcept;
        void* add_command(command_packet_key_t key, std::size_t size, std::size_t align, CommandType type);

        LinearAllocator command_allocator_;
        std::vector<CommandPacket> command_packets_;

        CommandBufferState state_ = CommandBufferState::Initial;
    };

    DEFINE_COMMAND(CopyBuffer, Graphics)
    {
        GPUBufferHandle dst;
        std::size_t dst_offset;
        GPUBufferHandle src;
        std::size_t src_offset;
        std::size_t size;
    };
    static_assert(IsValidCmdType<CmdCopyBuffer>::value);

    DEFINE_COMMAND(BeginRenderPass, Graphics)
    {
        RenderPassHandle render_pass;
        FramebufferHandle framebuffer;
        Vector2_u render_area;
        Vector4_f clear_color;
    };
    static_assert(IsValidCmdType<CmdBeginRenderPass>::value);

    DEFINE_COMMAND(EndRenderPass, Graphics){};
    static_assert(IsValidCmdType<CmdEndRenderPass>::value);

    DEFINE_COMMAND(Draw, Graphics)
    {
        DrawState draw_state;
        std::uint32_t vertex_count;
        std::uint32_t first_vertex;
    };
    static_assert(IsValidCmdType<CmdDraw>::value);

    DEFINE_COMMAND(DrawIndexed, Graphics)
    {
        DrawState draw_state;
        std::uint32_t vertex_offset;
        std::uint32_t index_offset;
        std::uint32_t index_count;
    };
    static_assert(IsValidCmdType<CmdDrawIndexed>::value);

    DEFINE_COMMAND(BindDescriptorSet, Any)
    {
        PipelineHandle pipeline;
        std::uint32_t binding;
        DescriptorSetHandle descriptor_set;
    };
    static_assert(IsValidCmdType<CmdBindDescriptorSet>::value);

    DEFINE_COMMAND(PipelineBarrier, Any)
    {
        PipelineStageFlags src_stages;
        PipelineStageFlags dst_stages;
        ImageBarrierDesc image_barrier;
    };
    static_assert(IsValidCmdType<CmdPipelineBarrier>::value);

    DEFINE_COMMAND(BlitImage, Transfer)
    {
        ImageHandle src_image;
        ImageLayout src_layout;
        Vector2_u src_size;
        ImageHandle dst_image;
        ImageLayout dst_layout;
        Vector2_u dst_size;
    };
    static_assert(IsValidCmdType<CmdBlitImage>::value);

    DEFINE_COMMAND(PushConstants, Any)
    {
        PipelineHandle pipeline;
        ShaderStageFlags shader_stages;
        std::size_t offset;
        std::size_t size;
        const void* data;
    };
    static_assert(IsValidCmdType<CmdPushConstants>::value);

    DEFINE_COMMAND(CopyBufferToImage, Transfer)
    {
        GPUBufferHandle src_buffer;
        ImageHandle dst_image;
        ImageLayout dst_image_layout;
        Vector3_u dst_image_size;
    };
    static_assert(IsValidCmdType<CmdCopyBufferToImage>::value);
} // namespace orion

#undef DEFINE_COMMAND
