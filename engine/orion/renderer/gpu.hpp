#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <utility>

#define GPU_DEFINE_BITFLAG(EnumType)                                                     \
    [[nodiscard]] constexpr EnumType operator|(EnumType lhs, EnumType rhs) noexcept      \
    {                                                                                    \
        return static_cast<EnumType>(std::to_underlying(lhs) | std::to_underlying(rhs)); \
    }                                                                                    \
    [[nodiscard]] constexpr EnumType operator&(EnumType lhs, EnumType rhs) noexcept      \
    {                                                                                    \
        return static_cast<EnumType>(std::to_underlying(lhs) & std::to_underlying(rhs)); \
    }

namespace orion
{
    namespace gpu
    {
        struct Device;
        struct Image;
        struct ImageView;
        struct Swapchain;
        struct CommandBuffer;
        struct CommandAllocator;

        enum class Queue {
            Graphics = 0,
            Compute,
            Transfer,
        };

        enum class Format {
            Undefined = 0,
            BGRA8_Unorm,
        };

        enum class PipelineStage : std::uint64_t {
            None                  = 0ULL,
            DrawIndirect          = 1ull < 0u,
            IndexInput            = 1ull < 1u,
            VertexShader          = 1ull < 2u,
            FragmentShader        = 1ull < 3u,
            DepthStencilTests     = 1ull < 4u,
            ColorAttachmentOutput = 1ull < 5u,
            ComputeShader         = 1ull < 6u,
            Transfer              = 1ull < 7u,
            Host                  = 1ull < 8u,
            AllCommands           = 1ull < 9u,
        };
        GPU_DEFINE_BITFLAG(PipelineStage)

        enum class Access : std::uint64_t {
            None                        = 0ULL,
            IndirectRead                = 1ull < 0u,
            IndexRead                   = 1ull < 1u,
            ShaderRead                  = 1ull < 2u,
            ShaderWrite                 = 1ull < 3u,
            ColorAttachmentRead         = 1ull < 4u,
            ColorAttachmentWrite        = 1ull < 5u,
            DepthStencilAttachmentRead  = 1ull < 6u,
            DepthStencilAttachmentWrite = 1ull < 7u,
            TransferRead                = 1ull < 8u,
            TransferWrite               = 1ull < 9u,
            HostRead                    = 1ull < 10u,
            HostWrite                   = 1ull < 11u,
        };
        GPU_DEFINE_BITFLAG(Access)

        enum class ImageLayout {
            Undefined = 0,
            ColorAttachment,
            DepthStencilAttachment,
            DepthStencilReadOnly,
            ShaderReadOnly,
            TransferSrc,
            TransferDst,
            Present,
        };

        struct SwapchainFrame {
            Image* image          = nullptr;
            ImageView* image_view = nullptr;

            std::uint32_t image_index   = 0u;
            std::uint32_t acquire_index = 0u;
        };

        struct ImageBarrier {
            Image* image             = nullptr;
            PipelineStage src_stages = PipelineStage::None;
            Access src_access        = Access::None;
            PipelineStage dst_stages = PipelineStage::None;
            Access dst_access        = Access::None;
            ImageLayout old_layout   = ImageLayout::Undefined;
            ImageLayout new_layout   = ImageLayout::Undefined;
        };

        struct Barrier {
            std::span<const ImageBarrier> image_barriers;
        };

        struct TimelinePoint {
            Queue queue         = Queue::Graphics;
            std::uint64_t value = 0ull;
        };

        struct QueueWait {
            TimelinePoint timeline_point = {};
            PipelineStage wait_stages    = PipelineStage::None;
        };

        struct Submit {
            std::span<const QueueWait> waits;
            std::span<const CommandBuffer* const> command_buffers;

            Swapchain* swapchain            = nullptr;
            SwapchainFrame* swapchain_frame = nullptr;
        };

        struct DeviceDesc {
            std::uint32_t device_idx = 0u;
        };

        struct SwapchainDesc {
            void* native_window_handle = nullptr;
            std::uint32_t width        = 0u;
            std::uint32_t height       = 0u;
            std::uint32_t image_count  = 0u;
            Format image_format        = Format::Undefined;
        };

        Device* create_device(const DeviceDesc& desc);
        void destroy_device(Device* device);

        Swapchain* create_swapchain(Device* device, const SwapchainDesc& desc);
        std::optional<SwapchainFrame> acquire_swapchain_frame(Device* device, Swapchain* swapchain);
        void present(Device* device, Swapchain* swapchain, const SwapchainFrame& swapchain_frame);
        void destroy_swapchain(Device* device, Swapchain* swapchain);

        CommandAllocator* create_command_allocator(Device* device, Queue queue);
        CommandBuffer* begin_command_buffer(Device* device, CommandAllocator* allocator);
        void reset_command_allocator(Device* device, CommandAllocator* allocator);
        void destroy_command_allocator(Device* device, CommandAllocator* allocator);

        void wait_timeline(Device* device, const TimelinePoint& timeline_point);
        void wait_idle(Device* device);

        void barrier(CommandBuffer* command_buffer, const Barrier& barrier);

        TimelinePoint submit(Device* device, Queue queue, const Submit& submit);

        [[nodiscard]] const char* backend_name();
        [[nodiscard]] const char* backend_version();
        [[nodiscard]] const char* device_name(const Device* device);
    } // namespace gpu
} // namespace orion

#undef GPU_DEFINE_BITFLAG
