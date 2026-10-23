#pragma once

#include <cstdint>
#include <optional>
#include <span>

namespace orion
{
    namespace gpu
    {
        struct Device;
        struct Image;
        struct ImageView;
        struct CommandAllocator;
        struct CommandBuffer;
        struct Swapchain;

        enum class Queue {
            Graphics = 0,
            Compute,
            Transfer,
        };

        enum class Format {
            Undefined = 0,

            RGBA8_Unorm,
            RGBA8_Srgb,

            BGRA8_Unorm,
            BGRA8_Srgb,
        };

        enum class PipelineStage : std::uint64_t {
            None                  = 0ULL,
            DrawIndirect          = 1ull << 0u,
            IndexInput            = 1ull << 1u,
            VertexShader          = 1ull << 2u,
            FragmentShader        = 1ull << 3u,
            DepthStencilTests     = 1ull << 4u,
            ColorAttachmentOutput = 1ull << 5u,
            ComputeShader         = 1ull << 6u,
            Transfer              = 1ull << 7u,
            Host                  = 1ull << 8u,
            AllCommands           = 1ull << 9u,
        };
        inline constexpr PipelineStage operator|(PipelineStage lhs, PipelineStage rhs)
        {
            return static_cast<PipelineStage>(static_cast<std::uint64_t>(lhs) | static_cast<std::uint64_t>(rhs));
        }
        inline constexpr PipelineStage operator&(PipelineStage lhs, PipelineStage rhs)
        {
            return static_cast<PipelineStage>(static_cast<std::uint64_t>(lhs) & static_cast<std::uint64_t>(rhs));
        }

        enum class Access : std::uint64_t {
            None                        = 0ULL,
            IndirectRead                = 1ull << 0u,
            IndexRead                   = 1ull << 1u,
            ShaderRead                  = 1ull << 2u,
            ShaderWrite                 = 1ull << 3u,
            ColorAttachmentRead         = 1ull << 4u,
            ColorAttachmentWrite        = 1ull << 5u,
            DepthStencilAttachmentRead  = 1ull << 6u,
            DepthStencilAttachmentWrite = 1ull << 7u,
            TransferRead                = 1ull << 8u,
            TransferWrite               = 1ull << 9u,
            HostRead                    = 1ull << 10u,
            HostWrite                   = 1ull << 11u,
        };
        inline constexpr Access operator|(Access lhs, Access rhs)
        {
            return static_cast<Access>(static_cast<std::uint64_t>(lhs) | static_cast<std::uint64_t>(rhs));
        }
        inline constexpr Access operator&(Access lhs, Access rhs)
        {
            return static_cast<Access>(static_cast<std::uint64_t>(lhs) & static_cast<std::uint64_t>(rhs));
        }

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

        struct SubresourceRange {
            std::uint32_t base_mip_level   = 0u;
            std::uint32_t level_count      = 1u;
            std::uint32_t base_array_layer = 0u;
            std::uint32_t layer_count      = 1u;
        };

        struct ImageBarrier {
            Image* image             = nullptr;
            PipelineStage src_stages = PipelineStage::None;
            Access src_access        = Access::None;
            PipelineStage dst_stages = PipelineStage::None;
            Access dst_access        = Access::None;
            ImageLayout old_layout   = ImageLayout::Undefined;
            ImageLayout new_layout   = ImageLayout::Undefined;
            SubresourceRange subresource_range;
        };

        struct Barrier {
            std::span<const ImageBarrier> image_barriers;
        };

        struct SwapchainFrame {
            Swapchain* swapchain  = nullptr;
            Image* image          = nullptr;
            ImageView* image_view = nullptr;

            std::uint32_t image_index   = 0u;
            std::uint32_t acquire_index = 0u;
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
            SwapchainFrame* swapchain_frame = nullptr;
        };

        struct DeviceDesc {
            std::uint32_t device_index = 0u;
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
        void present(Device* device, const SwapchainFrame& swapchain_frame);
        void destroy_swapchain(Device* device, Swapchain* swapchain);

        CommandAllocator* create_command_allocator(Device* device, Queue queue);
        CommandBuffer* begin_command_buffer(Device* device, CommandAllocator* allocator);
        void reset_command_allocator(Device* device, CommandAllocator* allocator);
        void destroy_command_allocator(Device* device, CommandAllocator* allocator);

        void barrier(CommandBuffer* command_buffer, const Barrier& barrier);
        TimelinePoint submit(Device* device, Queue queue, const Submit& submit);

        void wait_idle(Device* device);
        void wait_queue_idle(Device* device, Queue queue);
        void wait_timeline(Device* device, const TimelinePoint& timeline_point);

        [[nodiscard]] const char* backend_name();
        [[nodiscard]] const char* backend_version();
        [[nodiscard]] const char* device_name(const Device* device);
    } // namespace gpu
} // namespace orion
