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
    }                                                                                    \
    constexpr EnumType& operator&=(EnumType& lhs, EnumType rhs) noexcept                 \
    {                                                                                    \
        lhs = lhs & rhs;                                                                 \
        return lhs;                                                                      \
    }                                                                                    \
    constexpr EnumType& operator|=(EnumType& lhs, EnumType rhs) noexcept                 \
    {                                                                                    \
        lhs = lhs | rhs;                                                                 \
        return lhs;                                                                      \
    }

namespace orion
{
    namespace gpu
    {
        inline constexpr auto max_push_constants_size = 128u;
        inline constexpr auto max_color_attachments   = 8u;

        inline constexpr auto buffer_whole_size = static_cast<std::size_t>(~0ull);

        struct Device;
        struct Image;
        struct ImageView;
        struct Buffer;
        struct Swapchain;
        struct PipelineState;
        struct CommandBuffer;
        struct CommandAllocator;

        using Address = std::uint64_t;

        enum class Queue {
            Graphics = 0,
            Compute,
            Transfer,
        };

        enum class Memory {
            Default,
            Upload,
            Readback,
        };

        enum class Format {
            Undefined = 0,

            RGBA8_Unorm,
            RGBA8_Srgb,

            BGRA8_Unorm,
            BGRA8_Srgb,

            R32_Float,
            D32_Float,
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
        GPU_DEFINE_BITFLAG(PipelineStage)

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

        enum class LoadOp {
            Load = 0,
            Clear,
            DontCare,
        };

        enum class StoreOp {
            Store = 0,
            DontCare,
        };

        enum class ImageUsage : std::uint32_t {
            None                   = 0u,
            TransferSrc            = 1u << 0u,
            TransferDst            = 1u << 1u,
            Sampled                = 1u << 2u,
            Storage                = 1u << 3u,
            ColorAttachment        = 1u << 4u,
            DepthStencilAttachment = 1u << 5u,
        };
        GPU_DEFINE_BITFLAG(ImageUsage)

        enum class BufferUsage : std::uint32_t {
            None           = 0u,
            TransferSrc    = 1u << 0u,
            TransferDst    = 1u << 1u,
            IndexBuffer    = 1u << 2u,
            IndirectBuffer = 1u << 3u,
            DeviceAddress  = 1u << 4u,
        };
        GPU_DEFINE_BITFLAG(BufferUsage)

        struct ImageBarrier {
            Image* image             = nullptr;
            PipelineStage src_stages = PipelineStage::None;
            Access src_access        = Access::None;
            PipelineStage dst_stages = PipelineStage::None;
            Access dst_access        = Access::None;
            ImageLayout old_layout   = ImageLayout::Undefined;
            ImageLayout new_layout   = ImageLayout::Undefined;
        };

        struct BufferBarrier {
            Buffer* buffer           = nullptr;
            PipelineStage src_stages = PipelineStage::None;
            Access src_access        = Access::None;
            PipelineStage dst_stages = PipelineStage::None;
            Access dst_access        = Access::None;
            std::size_t size         = 0ull;
            std::size_t offset       = 0ull;
        };

        struct Barrier {
            std::span<const ImageBarrier> image_barriers;
            std::span<const BufferBarrier> buffer_barriers;
        };

        struct Viewport {
            float x         = 0.0f;
            float y         = 0.0f;
            float width     = 0.0f;
            float height    = 0.0f;
            float min_depth = 0.0f;
            float max_depth = 1.0f;
        };

        struct Offset2D {
            std::int32_t x = 0;
            std::int32_t y = 0;
        };

        struct Offset3D {
            std::int32_t x = 0;
            std::int32_t y = 0;
            std::int32_t z = 0;
        };

        struct Extent2D {
            std::uint32_t width  = 0u;
            std::uint32_t height = 0u;
        };

        struct Extent3D {
            std::uint32_t width  = 0u;
            std::uint32_t height = 0u;
            std::uint32_t depth  = 0u;
        };

        struct Rect2D {
            Offset2D offset = {};
            Extent2D extent = {};
        };

        struct ClearColorValue {
            float float32[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        };

        struct ClearDepthStencilValue {
            float depth           = 0.0f;
            std::uint32_t stencil = 0u;
        };

        struct ClearValue {
            union {
                ClearColorValue color;
                ClearDepthStencilValue depth_stencil;
            };
        };

        struct RenderPassAttachment {
            ImageView* image_view  = nullptr;
            LoadOp load_op         = LoadOp::Load;
            StoreOp store_op       = StoreOp::Store;
            ClearValue clear_value = {};
        };

        struct RenderPass {
            std::span<const RenderPassAttachment> color_attachments;
            std::optional<RenderPassAttachment> depth_attachment;
        };

        struct Draw {
            std::uint32_t vertex_count   = 0u;
            std::uint32_t instance_count = 1u;
            std::uint32_t first_vertex   = 0u;
            std::uint32_t first_instance = 0u;
        };

        struct DrawIndexed {
            std::uint32_t index_count    = 0u;
            std::uint32_t instance_count = 1u;
            std::uint32_t first_index    = 0u;
            std::int32_t vertex_offset   = 0;
            std::uint32_t first_instance = 0u;
        };

        struct BufferImageCopy {
            std::size_t buffer_offset            = 0ull;
            std::uint32_t buffer_row_length      = 0u;
            std::uint32_t buffer_image_height    = 0u;
            std::uint32_t image_mip_level        = 0u;
            std::uint32_t image_base_array_layer = 0u;
            std::uint32_t image_layer_count      = 1u;
            Offset3D image_offset                = {};
            Extent3D image_extent                = {};
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

        struct ImageDesc {
            std::uint32_t width        = 0u;
            std::uint32_t height       = 0u;
            std::uint32_t depth        = 1u;
            Format format              = Format::Undefined;
            std::uint32_t mip_levels   = 1u;
            std::uint32_t array_layers = 1u;
            ImageUsage usage           = ImageUsage::None;
        };

        struct ImageViewDesc {
            Image* image                   = nullptr;
            std::uint32_t base_mip_level   = 0u;
            std::uint32_t level_count      = 1u;
            std::uint32_t base_array_layer = 0u;
            std::uint32_t layer_count      = 1u;
        };

        struct BufferDesc {
            std::size_t size  = 0ull;
            Memory memory     = Memory::Default;
            BufferUsage usage = BufferUsage::None;
        };

        Device* create_device(const DeviceDesc& desc);
        void destroy_device(Device* device);

        Swapchain* create_swapchain(Device* device, const SwapchainDesc& desc);
        void resize_swapchain(Device* device, Swapchain* swapchain, std::uint32_t width, std::uint32_t height);
        std::optional<SwapchainFrame> acquire_swapchain_frame(Device* device, Swapchain* swapchain);
        void present(Device* device, Swapchain* swapchain, const SwapchainFrame& swapchain_frame);
        void destroy_swapchain(Device* device, Swapchain* swapchain);

        CommandAllocator* create_command_allocator(Device* device, Queue queue);
        CommandBuffer* begin_command_buffer(Device* device, CommandAllocator* allocator);
        void reset_command_allocator(Device* device, CommandAllocator* allocator);
        void destroy_command_allocator(Device* device, CommandAllocator* allocator);

        PipelineState* create_graphics_pipeline(Device* device, const GraphicsPipelineDesc& desc);
        void destroy_pipeline_state(Device* device, PipelineState* pipeline_state);

        Image* create_image(Device* device, const ImageDesc& desc);
        [[nodiscard]] Extent3D get_image_extent(const Image* image);
        [[nodiscard]] Extent2D get_image_extent_2d(const Image* image);
        void destroy_image(Device* device, Image* image);

        ImageView* create_image_view(Device* device, const ImageViewDesc& desc);
        void set_shader_resource(Device* device, ImageView* image_view, std::uint32_t index);
        void destroy_image_view(Device* device, ImageView* image_view);

        Buffer* create_buffer(Device* device, const BufferDesc& desc);
        void copy_to_buffer(Device* device, Buffer* dst, std::size_t dst_offset, std::span<const std::byte> src);
        void copy_from_buffer(Device* device, Buffer* src, std::size_t src_offset, std::span<std::byte> dst);
        [[nodiscard]] Address get_buffer_device_address(const Buffer* buffer);
        void destroy_buffer(Device* device, Buffer* buffer);

        void wait_timeline(Device* device, const TimelinePoint& timeline_point);
        void wait_idle(Device* device);

        void barrier(CommandBuffer* command_buffer, const Barrier& barrier);
        void bind_pso(CommandBuffer* command_buffer, PipelineState* pipeline_state);
        void bind_index_buffer(CommandBuffer* command_buffer, Buffer* index_buffer);
        void set_viewport(CommandBuffer* command_buffer, const Viewport& viewport);
        void set_scissor(CommandBuffer* command_buffer, const Rect2D& scissor);
        void begin_render_pass(CommandBuffer* command_buffer, const RenderPass& render_pass);
        void end_render_pass(CommandBuffer* command_buffer);
        void draw(CommandBuffer* command_buffer, const Draw& draw);
        void draw_indexed(CommandBuffer* command_buffer, const DrawIndexed& draw);
        void push_constants(CommandBuffer* command_buffer, std::uint32_t offset, std::span<const std::byte> data);
        void copy_buffer_to_image(CommandBuffer* command_buffer, Buffer* src, Image* dst, const BufferImageCopy& copy);

        TimelinePoint submit(Device* device, Queue queue, const Submit& submit);

        void debug_marker_begin(CommandBuffer* command_buffer, const char* name);
        void debug_marker_end(CommandBuffer* command_buffer);

        [[nodiscard]] const char* backend_name();
        [[nodiscard]] const char* backend_version();
        [[nodiscard]] const char* device_name(const Device* device);
    } // namespace gpu
} // namespace orion

// ImGui integration
struct ImGui_ImplOrion_InitInfo {
    void* native_window_handle = nullptr;
    orion::gpu::Device* device = nullptr;
    std::uint32_t image_count  = 0u;
    std::span<const orion::gpu::Format> color_attachment_formats;
};
bool ImGui_ImplOrion_Init(const ImGui_ImplOrion_InitInfo& init_info);
void ImGui_ImplOrion_NewFrame();
void ImGui_ImplOrion_RenderDrawData(struct ImDrawData* draw_data, orion::gpu::CommandBuffer* command_buffer);
void ImGui_ImplOrion_Shutdown();

#undef GPU_DEFINE_BITFLAG
