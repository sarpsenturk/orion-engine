#pragma once

#include "barrier.h"
#include "handles.h"
#include "types.h"

#include <orion-utils/allocators/linear_allocator.h>

#include <orion-math/vector/vector2.h>
#include <orion-math/vector/vector4.h>

#include <spdlog/logger.h>

#include <span>
#include <type_traits>
#include <vector>

namespace orion
{
    struct CommandPoolDesc {
        CommandQueueType queue_type;
    };

    struct CommandBufferBeginDesc {
        CommandBufferUsageFlags usage;
    };

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

    struct CommandPacket {
        std::uint64_t key;
        CommandType type;
        const void* data;
    };

    struct CommandBufferDesc {
        CommandPoolHandle command_pool;
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
        CommandList(RenderDevice* device, CommandBufferHandle command_buffer, std::size_t size);

        template<typename Command>
            requires(is_valid_cmd_type<Command>::value)
        auto* add_command(std::uint64_t key)
        {
            return static_cast<Command*>(add_command(key, sizeof(Command), alignof(Command), Command::type));
        }

        [[nodiscard]] auto command_buffer() const noexcept { return command_buffer_; }

        [[nodiscard]] auto state() const noexcept { return state_; }
        [[nodiscard]] bool is_recording() const noexcept { return state_ == CommandBufferState::Recording; }
        [[nodiscard]] bool is_executable() const noexcept { return state_ == CommandBufferState::Executable; }
        [[nodiscard]] bool is_pending() const noexcept { return state_ == CommandBufferState::Pending; }
        [[nodiscard]] bool is_invalid() const noexcept { return state_ == CommandBufferState::Invalid; }

        void begin(const CommandBufferBeginDesc& desc);
        void end();
        void reset();

        // Flush commands from CPU side to GPU side
        void flush();
        // Clear commands from CPU side without flushing to GPU side
        void clear();

    private:
        void set_state(CommandBufferState state) noexcept;
        void* add_command(std::uint64_t key, std::size_t size, std::size_t align, CommandType type);

        static spdlog::logger* logger();

        RenderDevice* device_;
        CommandBufferHandle command_buffer_;
        LinearAllocator command_allocator_;
        std::vector<CommandPacket> command_packets_;

        CommandBufferState state_ = CommandBufferState::Initial;
    };

    DEFINE_COMMAND(BufferCopy, Graphics)
    {
        GPUBufferHandle dst = GPUBufferHandle::invalid_handle();
        std::size_t dst_offset = 0;
        GPUBufferHandle src = GPUBufferHandle::invalid_handle();
        std::size_t src_offset = 0;
        std::size_t size = 0;
    };

    DEFINE_COMMAND(BeginRenderPass, Graphics)
    {
        RenderPassHandle render_pass = RenderPassHandle::invalid_handle();
        FramebufferHandle framebuffer = FramebufferHandle::invalid_handle();
        Vector2_u render_area = {};
        Vector4_f clear_color = {};
    };

    DEFINE_COMMAND(EndRenderPass, Graphics){};

    DEFINE_COMMAND(Draw, Graphics)
    {
        GPUBufferHandle vertex_buffer = GPUBufferHandle::invalid_handle();
        PipelineHandle graphics_pipeline = PipelineHandle::invalid_handle();
        Viewport viewport = {};
        std::uint32_t vertex_count = 0;
        std::uint32_t first_vertex = 0;
    };

    DEFINE_COMMAND(DrawIndexed, Graphics)
    {
        GPUBufferHandle vertex_buffer = GPUBufferHandle::invalid_handle();
        GPUBufferHandle index_buffer = GPUBufferHandle::invalid_handle();
        IndexType index_type = IndexType::Uint32;
        PipelineHandle graphics_pipeline = PipelineHandle::invalid_handle();
        Viewport viewport = {};
        Scissor scissor = {};
        std::uint32_t vertex_offset = 0;
        std::uint32_t index_offset = 0;
        std::uint32_t index_count = 0;
    };

    DEFINE_COMMAND(BindDescriptorSets, Any)
    {
        PipelineHandle pipeline = PipelineHandle::invalid_handle();
        std::uint32_t first_set = 0;
        std::span<DescriptorSetHandle> descriptor_sets = {};
    };

    DEFINE_COMMAND(PipelineBarrier, Any)
    {
        PipelineStageFlags src_stages;
        PipelineStageFlags dst_stages;
        std::span<const ImageBarrierDesc> image_barriers;
    };

    DEFINE_COMMAND(BlitImage, Transfer)
    {
        ImageHandle src_image;
        ImageLayout src_layout;
        Vector2_u src_size;
        ImageHandle dst_image;
        ImageLayout dst_layout;
        Vector2_u dst_size;
    };
} // namespace orion

#undef DEFINE_COMMAND
