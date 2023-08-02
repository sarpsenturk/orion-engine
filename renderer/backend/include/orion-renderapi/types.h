#pragma once

#include "handles.h"

#include "orion-math/vector/vector2.h"
#include "orion-math/vector/vector3.h"

#include "orion-utils/bitflag.h"

#include <cstdint>
#include <span>
#include <string>

namespace orion
{
    enum class Format : std::uint32_t {
        Undefined,
        R8_Unorm,
        B8G8R8A8_Srgb,
        R8G8B8A8_Unorm,
        R32G32_Float,
        R32G32B32_Float,
        R32G32B32A32_Float,
    };

    constexpr auto size_of(Format format) -> std::uint32_t
    {
        switch (format) {
            case Format::Undefined:
                break;
            case Format::B8G8R8A8_Srgb:
            case Format::R8G8B8A8_Unorm:
                return sizeof(uint8_t) * 4;
            case Format::R32G32_Float:
                return sizeof(float) * 2;
            case Format::R32G32B32_Float:
                return sizeof(float) * 3;
            case Format::R32G32B32A32_Float:
                return sizeof(float) * 4;
        }
        return UINT32_MAX;
    }

    constexpr auto format_as(Format format) noexcept -> const char*
    {
        switch (format) {
            case Format::Undefined:
                return "Undefined";
            case Format::B8G8R8A8_Srgb:
                return "B8G8R8A8_Srgb";
            case Format::R32G32_Float:
                return "R32G32_Float";
            case Format::R32G32B32_Float:
                return "R32G32B32_Float";
            case Format::R32G32B32A32_Float:
                return "R32G32B32A32_Float";
        }
        return "Unknown format";
    }

    enum class PhysicalDeviceType {
        Other,
        Integrated,
        Discrete,
        Virtual,
        CPU
    };

    constexpr auto format_as(PhysicalDeviceType type) noexcept
    {
        switch (type) {
            case PhysicalDeviceType::Other:
                break;
            case PhysicalDeviceType::Integrated:
                return "Integrated";
            case PhysicalDeviceType::Discrete:
                return "Discrete";
            case PhysicalDeviceType::Virtual:
                return "Virtual";
            case PhysicalDeviceType::CPU:
                return "CPU";
        }
        return "Other";
    }

    enum class AttachmentLoadOp {
        Load,
        Clear,
        DontCare
    };

    enum class AttachmentStoreOp {
        Store,
        DontCare
    };

    enum class ImageLayout {
        Undefined = 0,
        ColorAttachment,
        PresentSrc,
        TransferSrc,
        TransferDst,
        ShaderReadOnly
    };

    enum class ShaderStage : std::uint8_t {
        Vertex,
        Fragment
    };
    using ShaderStageFlags = Bitflag<ShaderStage>;

    enum class PrimitiveTopology {
        TriangleList
    };

    enum class FillMode {
        Solid,
        Wireframe,
    };

    enum class CullMode {
        None,
        Front,
        Back,
        FrontAndBack
    };

    enum class FrontFace {
        CounterClockWise,
        ClockWise
    };

    enum class InputRate {
        Vertex,
        Instance
    };

    enum class GPUBufferUsage : std::uint8_t {
        VertexBuffer,
        IndexBuffer,
        ConstantBuffer,
        TransferSrc,
        TransferDst
    };
    using GPUBufferUsageFlags = Bitflag<GPUBufferUsage>;

    inline constexpr auto gpu_buffer_usage_transfer_flags =
        GPUBufferUsageFlags::disjunction({GPUBufferUsage::TransferSrc, GPUBufferUsage::TransferDst});

    struct Viewport {
        Vector2_f position;
        Vector2_f size;
        Vector2_f depth;
    };

    struct Scissor {
        Vector2_i offset;
        Vector2_u size;
    };

    enum class CommandQueueType {
        Graphics,
        Transfer,
        Compute,
        Any
    };

    enum class CommandType {
        BufferCopy,
        BeginRenderPass,
        EndRenderPass,
        Draw,
        DrawIndexed,
        BindDescriptorSet,
        PipelineBarrier,
        BlitImage,
        PushConstants,
        CopyBufferToImage
    };

    enum class DescriptorType : std::uint8_t {
        Unknown,
        ConstantBuffer,
        ImageSampler,
        SampledImage
    };

    enum class ShaderObjectType {
        SpirV,
        DXIL
    };

    enum class CommandBufferUsage : std::uint8_t {
        OneTimeSubmit
    };
    using CommandBufferUsageFlags = Bitflag<CommandBufferUsage>;

    enum class ImageType {
        Image1D,
        Image2D,
        Image3D
    };

    enum class ImageTiling {
        Optimal,
        Linear
    };

    enum class ImageUsage : std::uint8_t {
        TransferSrc,
        TransferDst,
        ColorAttachment,
        DepthStencilAttachment,
        InputAttachment,
        SampledImage
    };
    using ImageUsageFlags = Bitflag<ImageUsage>;

    inline constexpr auto image_usage_transfer_flags =
        ImageUsageFlags::disjunction({ImageUsage::TransferSrc, ImageUsage::TransferDst});

    enum class ImageViewType {
        View1D,
        View2D,
        View3D,
        ViewCube,
        View1DArray,
        View2DArray,
        ViewCubeArray,
    };

    enum class ResourceAccess : std::uint8_t {
        ColorAttachmentWrite,
        TransferRead,
        TransferWrite,
        MemoryRead,
        MemoryWrite,
        ShaderRead,
        ShaderWrite
    };
    using ResourceAccessFlags = Bitflag<ResourceAccess>;

    enum class PipelineStage : std::uint8_t {
        TopOfPipe,
        ColorAttachmentOutput,
        Transfer,
        BottomOfPipe,
        FragmentShader
    };
    using PipelineStageFlags = Bitflag<PipelineStage>;

    enum class IndexType {
        Uint16,
        Uint32,
    };

    enum class Filter {
        Nearest,
        Linear
    };

    enum class AddressMode {
        Repeat,
        Mirror,
        Clamp,
        Border
    };

    enum class CompareFunc {
        Never,
        Less,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };

    // Only format is needed except when creating render passes
    struct AttachmentDesc {
        Format format;
        AttachmentLoadOp load_op = AttachmentLoadOp::DontCare;
        AttachmentStoreOp store_op = AttachmentStoreOp::DontCare;
        ImageLayout initial_layout = ImageLayout::Undefined;
        ImageLayout layout = ImageLayout::Undefined;
        ImageLayout final_layout = ImageLayout::Undefined;
    };

    struct AttachmentList {
        std::span<const AttachmentDesc> color_attachments;

        [[nodiscard]] auto attachment_count() const noexcept
        {
            return color_attachments.size();
        }
    };

    struct ImageBarrierDesc {
        ResourceAccessFlags src_access;
        ResourceAccessFlags dst_access;
        ImageLayout old_layout;
        ImageLayout new_layout;
        ImageHandle image;
    };

    struct GPUBufferDesc {
        std::size_t size = 0;
        GPUBufferUsageFlags usage = {};
        bool host_visible = false;
    };

    struct CommandPoolDesc {
        CommandQueueType queue_type;
    };

    struct CommandBufferDesc {
        CommandPoolHandle command_pool;
    };

    struct CommandBufferBeginDesc {
        CommandBufferUsageFlags usage;
    };

    struct DescriptorPoolSize {
        DescriptorType type;
        std::uint32_t count;
    };

    struct DescriptorBinding {
        DescriptorType type;
        ShaderStageFlags shader_stages;
        std::uint32_t count;
    };

    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout(std::initializer_list<DescriptorBinding> bindings);

        [[nodiscard]] auto& bindings() const noexcept { return bindings_; }
        [[nodiscard]] auto hash() const noexcept { return hash_; }

    private:
        static std::size_t hash_bindings(std::span<const DescriptorBinding> bindings);

        std::vector<DescriptorBinding> bindings_;
        std::size_t hash_;
    };

    struct DescriptorPoolDesc {
        std::uint32_t max_sets;
        std::span<const DescriptorPoolSize> pool_sizes;
    };

    struct DescriptorSetDesc {
        DescriptorPoolHandle descriptor_pool;
        const DescriptorSetLayout* layout;
    };

    struct DescriptorSetUpdate {
        DescriptorSetHandle descriptor_set;
        std::uint32_t binding;
        DescriptorType descriptor_type;

        GPUBufferHandle buffer_handle;
        std::size_t buffer_offset;
        std::size_t buffer_size;

        ImageViewHandle image_view;
        ImageLayout image_layout = ImageLayout::Undefined;

        SamplerHandle sampler;
    };

    struct PushConstantDesc {
        std::size_t size;
        ShaderStageFlags shader_stages;
    };

    struct FramebufferDesc {
        AttachmentList attachment_list;
        std::span<const ImageViewHandle> image_views = {};
        Vector2_u size = {};
    };

    struct ImageDesc {
        ImageType type;
        Format format;
        Vector3_u size;
        ImageTiling tiling;
        ImageUsageFlags usage;
    };

    struct ImageViewDesc {
        ImageHandle image;
        ImageViewType type;
        Format format;
    };

    struct ShaderStageDesc {
        ShaderModuleHandle module = ShaderModuleHandle::invalid_handle();
        ShaderStage stage = {};
        const char* entry_point = "main";
    };

    struct VertexAttributeDesc {
        const char* name = nullptr;
        Format format = {};
        std::uint32_t offset = UINT32_MAX;
    };

    class VertexBinding
    {
    public:
        constexpr VertexBinding(std::span<const VertexAttributeDesc> attributes, InputRate input_rate)
            : attributes_(attributes.begin(), attributes.end())
            , input_rate_(input_rate)
            , stride_(calculate_stride_and_offsets())
        {
        }

        constexpr VertexBinding(std::initializer_list<VertexAttributeDesc> attributes, InputRate input_rate)
            : attributes_(attributes)
            , input_rate_(input_rate)
            , stride_(calculate_stride_and_offsets())
        {
        }

        [[nodiscard]] constexpr auto& attributes() const noexcept { return attributes_; }
        [[nodiscard]] constexpr auto input_rate() const noexcept { return input_rate_; }
        [[nodiscard]] constexpr auto stride() const noexcept { return stride_; }

    private:
        constexpr std::uint32_t calculate_stride_and_offsets() noexcept
        {
            std::uint32_t offset = 0;
            for (auto& attribute : attributes_) {
                attribute.offset = offset;
                offset += size_of(attribute.format);
            }
            return offset;
        }

        std::vector<VertexAttributeDesc> attributes_;
        InputRate input_rate_;
        std::uint32_t stride_;
    };

    struct InputAssemblyDesc {
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    };

    struct RasterizationDesc {
        FillMode fill_mode = FillMode::Solid;
        CullMode cull_mode = CullMode::Back;
        FrontFace front_face = FrontFace::ClockWise;
    };

    struct GraphicsPipelineDesc {
        std::span<const ShaderStageDesc> shaders = {};
        std::span<const VertexBinding> vertex_bindings = {};
        std::span<const DescriptorSetLayout> descriptor_layouts = {};
        std::span<const PushConstantDesc> push_constants = {};
        InputAssemblyDesc input_assembly = {};
        RasterizationDesc rasterization = {};
        AttachmentList attachment_list;
    };

    struct RenderPassDesc {
        AttachmentList attachments;
    };

    struct ShaderModuleDesc {
        std::span<const std::byte> byte_code;
    };

    inline constexpr auto default_swapchain_image_count = 2;
    inline constexpr auto default_swapchain_format = Format::B8G8R8A8_Srgb;

    struct SwapchainDesc {
        SurfaceHandle surface;
        std::uint32_t image_count = default_swapchain_image_count;
        Format image_format = default_swapchain_format;
        Vector2_u image_size = {};
        ImageUsageFlags image_usage = ImageUsage::ColorAttachment;
    };

    struct SwapchainAttachmentDesc {
        SwapchainHandle swapchain = SwapchainHandle::invalid_handle();
        Format format = default_swapchain_format;
    };

    struct SwapchainPresentDesc {
        SwapchainHandle swapchain = SwapchainHandle::invalid_handle();
        SemaphoreHandle wait_semaphore = SemaphoreHandle::invalid_handle();
        std::uint32_t image_index = UINT32_MAX;
    };

    struct SamplerDesc {
        Filter filter;
        AddressMode address_mode_u;
        AddressMode address_mode_v;
        AddressMode address_mode_w;
        float mip_load_bias;
        float max_anisotropy;
        CompareFunc compare_func;
        float min_lod;
        float max_lod;
    };
} // namespace orion
