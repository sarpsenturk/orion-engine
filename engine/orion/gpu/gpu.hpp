#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace orion
{
    namespace gpu
    {
        inline constexpr auto max_color_attachments = 8u;

        struct Device;
        struct Image;
        struct ImageView;
        struct Buffer;
        struct PipelineState;
        struct CommandAllocator;
        struct CommandBuffer;
        struct Swapchain;

        enum class Queue {
            Graphics = 0,
            Compute,
            Transfer,
        };

        enum class Memory {
            GpuOnly,
            CpuVisible,
            Readback,
        };

        using Address = std::uint64_t;

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

        enum class LoadOp {
            Load = 0,
            Clear,
            DontCare,
        };

        enum class StoreOp {
            Store = 0,
            DontCare,
        };

        enum class Topology {
            TriangleList = 0,
        };

        enum class CullMode {
            None = 0,
            Front,
            Back,
        };

        enum class CompareOp {
            Never = 0,
            Less,
            Equal,
            LessOrEqual,
            Greater,
            NotEqual,
            GreaterOrEqual,
            Always,
        };

        struct Offset2D {
            std::int32_t x = 0;
            std::int32_t y = 0;
        };

        struct Extent2D {
            std::uint32_t width  = 0u;
            std::uint32_t height = 0u;
        };

        struct Rect2D {
            Offset2D offset = {};
            Extent2D extent = {};
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

        struct RenderPassAttachment {
            ImageView* image_view = nullptr;
            LoadOp load_op        = LoadOp::Load;
            StoreOp store_op      = StoreOp::Store;
        };

        struct RenderPass {
            std::span<const RenderPassAttachment> color_attachments;
            std::optional<RenderPassAttachment> depth_attachment;
        };

        struct Viewport {
            float x         = 0.0f;
            float y         = 0.0f;
            float width     = 0.0f;
            float height    = 0.0f;
            float min_depth = 0.0f;
            float max_depth = 1.0f;
        };

        struct Draw {
            std::uint32_t vertex_count   = 0u;
            std::uint32_t instance_count = 1u;
            std::uint32_t first_vertex   = 0u;
            std::uint32_t first_instance = 0u;
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

        struct RasterizerDesc {
            CullMode cull_mode  = CullMode::None;
            bool front_face_ccw = false;
        };

        struct DepthStencilDesc {
            bool depth_test_enable     = false;
            bool depth_write_enable    = false;
            CompareOp depth_compare_op = CompareOp::Never;
        };

        struct GraphicsPipelineDesc {
            std::span<const std::uint32_t> spirv;
            Topology topology              = Topology::TriangleList;
            RasterizerDesc rasterizer      = {};
            DepthStencilDesc depth_stencil = {};
            std::span<const Format> color_attachments;
            Format depth_attachment = Format::Undefined;
        };

        struct BufferDesc {
            std::size_t size = 0z;
            Memory memory    = Memory::GpuOnly;
        };

        Device* create_device(const DeviceDesc& desc);
        void destroy_device(Device* device);

        Swapchain* create_swapchain(Device* device, const SwapchainDesc& desc);
        void resize_swapchain(Device* device, Swapchain* swapchain, std::uint32_t width, std::uint32_t height);
        std::optional<SwapchainFrame> acquire_swapchain_frame(Device* device, Swapchain* swapchain);
        void present(Device* device, const SwapchainFrame& swapchain_frame);
        void destroy_swapchain(Device* device, Swapchain* swapchain);

        CommandAllocator* create_command_allocator(Device* device, Queue queue);
        CommandBuffer* begin_command_buffer(Device* device, CommandAllocator* allocator);
        void reset_command_allocator(Device* device, CommandAllocator* allocator);
        void destroy_command_allocator(Device* device, CommandAllocator* allocator);

        PipelineState* create_graphics_pipeline(Device* device, const GraphicsPipelineDesc& desc);
        void destroy_pipeline(Device* device, PipelineState* pipeline);

        Extent2D image_extent_2d(const Image* image);

        Buffer* create_buffer(Device* device, const BufferDesc& desc);
        // Returns nullptr for GpuOnly buffers; CpuVisible and Readback buffers are mapped and host-coherent.
        void* buffer_host_address(const Buffer* buffer);
        Address buffer_device_address(const Buffer* buffer);
        void destroy_buffer(Device* device, Buffer* buffer);

        void set_viewport(CommandBuffer* command_buffer, const Viewport& viewport);
        void set_scissor(CommandBuffer* command_buffer, const Rect2D& scissor);
        void draw(CommandBuffer* command_buffer, const Draw& draw);
        void bind_pipeline_state(CommandBuffer* command_buffer, PipelineState* pipeline_state);
        void begin_render_pass(CommandBuffer* command_buffer, const RenderPass& render_pass);
        void end_render_pass(CommandBuffer* command_buffer);
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
