#pragma once

#include "handles.h"

#include "orion-math/vector/vector2.h"
#include "orion-math/vector/vector3.h"
#include "orion-math/vector/vector4.h"

#include "orion-utils/bitflag.h"
#include "orion-utils/static_vector.h"

#include <cstdint>
#include <span>
#include <string>
#include <variant>

namespace orion
{
    enum class PhysicalDeviceType {
        Other,
        Integrated,
        Discrete,
        Virtual,
        CPU
    };

    using physical_device_index_t = std::int32_t;
    inline constexpr auto invalid_physical_device_index = -1;

    struct PhysicalDeviceDesc {
        physical_device_index_t index;
        PhysicalDeviceType type;
        std::string name;
    };

    using SelectPhysicalDeviceFn = physical_device_index_t (*)(std::span<const PhysicalDeviceDesc>);

    enum class Format : std::uint32_t {
        Undefined,
        R8_Unorm,
        B8G8R8A8_Srgb,
        R8G8B8A8_Unorm,
        R32G32_Float,
        R32G32B32_Float,
        R32G32B32A32_Float,
    };

    constexpr auto format_size(Format format) -> std::uint32_t
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
            case Format::R8_Unorm:
                return sizeof(std::uint8_t);
        }
        return UINT32_MAX;
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
        General,
        ColorAttachment,
        PresentSrc,
        TransferSrc,
        TransferDst,
        ShaderReadOnly
    };

    ORION_BITFLAG(ShaderStageFlags, std::uint8_t){
        Vertex = 0x1,
        Pixel = 0x2};

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

    ORION_BITFLAG(GPUBufferUsageFlags, std::uint8_t){
        VertexBuffer = 0x1,
        IndexBuffer = 0x2,
        ConstantBuffer = 0x4,
        TransferSrc = 0x8,
        TransferDst = 0x10,
        StorageBuffer = 0x20,
        Transfer = TransferSrc | TransferDst};

    struct Viewport {
        Vector2_f position;
        Vector2_f size;
        Vector2_f depth;

        friend constexpr bool operator==(const Viewport& lhs, const Viewport& rhs) noexcept = default;
    };

    struct Scissor {
        Vector2_i offset;
        Vector2_u size;

        friend constexpr bool operator==(const Scissor& lhs, const Scissor& rhs) noexcept = default;
    };

    enum class CommandQueueType {
        Graphics,
        Transfer,
        Compute,
        Any
    };

    enum class BindingType : std::uint8_t {
        ConstantBuffer,
        StorageBuffer,
        SampledImage,
        Sampler
    };

    BindingType get_binding_type(GPUBufferUsageFlags buffer_usage);

    struct DescriptorBindingDesc {
        BindingType type;
        ShaderStageFlags shader_stages;
        std::uint32_t count;

        [[nodiscard]] std::size_t hash() const;
    };
    static_assert(sizeof(DescriptorBindingDesc) == sizeof(std::size_t));

    struct DescriptorLayoutDesc {
        std::span<const DescriptorBindingDesc> bindings;

        [[nodiscard]] std::size_t hash() const;
    };

    inline constexpr auto buffer_whole_size = SIZE_MAX;

    struct BufferRegion {
        std::size_t size = buffer_whole_size;
        std::size_t offset = 0ull;
    };

    struct BufferBindingDesc {
        GPUBufferHandle buffer_handle = GPUBufferHandle::invalid();
        BufferRegion region;
    };

    struct ImageBindingDesc {
        ImageViewHandle image_view_handle = ImageViewHandle::invalid();
        ImageLayout image_layout = ImageLayout::Undefined;
        SamplerHandle sampler_handle = SamplerHandle::invalid();
    };

    using DescriptorBindingValue = std::variant<BufferBindingDesc, ImageBindingDesc>;

    struct DescriptorBinding {
        std::uint32_t binding;
        BindingType binding_type;
        DescriptorBindingValue binding_value;
    };

    [[nodiscard]] constexpr bool is_buffer_binding(const DescriptorBinding& binding)
    {
        return std::holds_alternative<BufferBindingDesc>(binding.binding_value);
    }

    [[nodiscard]] constexpr bool is_image_binding(const DescriptorBinding& binding)
    {
        return std::holds_alternative<ImageBindingDesc>(binding.binding_value);
    }

    struct PushConstantDesc {
        std::uint32_t size;
        ShaderStageFlags shader_stages;
    };

    struct PipelineLayoutDesc {
        std::span<const DescriptorLayoutHandle> descriptors;
        std::span<const PushConstantDesc> push_constants;
    };

    enum class ShaderObjectType {
        SpirV,
        DXIL
    };

    enum class ImageType {
        Image1D,
        Image2D,
        Image3D
    };

    enum class ImageTiling {
        Optimal,
        Linear
    };

    ORION_BITFLAG(ImageUsageFlags, std::uint8_t){
        TransferSrc = 0x1,
        TransferDst = 0x2,
        ColorAttachment = 0x4,
        DepthStencilAttachment = 0x8,
        InputAttachment = 0x10,
        SampledImage = 0x20,
        Transfer = TransferSrc | TransferDst};

    enum class ImageViewType {
        View1D,
        View2D,
        View3D,
        ViewCube,
        View1DArray,
        View2DArray,
        ViewCubeArray,
    };

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

    struct ImageBarrierDesc {
        ImageLayout old_layout;
        ImageLayout new_layout;
        ImageHandle image;
    };

    struct GPUBufferDesc {
        std::size_t size = 0;
        GPUBufferUsageFlags usage = {};
        bool host_visible = false;
    };

    struct FramebufferDesc {
        RenderPassHandle render_pass;
        std::span<const ImageViewHandle> image_views;
        Vector2_u size;
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
        ShaderModuleHandle module = ShaderModuleHandle::invalid();
        ShaderStageFlags stage = {};
        const char* entry_point = "main";
    };

    struct VertexAttributeDesc {
        const char* name;
        Format format;
        std::uint32_t offset;
    };

    inline constexpr VertexAttributeDesc vertex_attr(const char* name, Format format)
    {
        return {name, format};
    }

    class VertexBinding
    {
    public:
        static constexpr auto max_attribute_count = 6;

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

        [[nodiscard]] constexpr auto attributes() const noexcept { return std::span{attributes_}; }
        [[nodiscard]] constexpr auto input_rate() const noexcept { return input_rate_; }
        [[nodiscard]] constexpr auto stride() const noexcept { return stride_; }

    private:
        constexpr std::uint32_t calculate_stride_and_offsets() noexcept
        {
            std::uint32_t offset = 0;
            for (auto& attribute : attributes_) {
                attribute.offset = offset;
                offset += format_size(attribute.format);
            }
            return offset;
        }

        std::vector<VertexAttributeDesc> attributes_;
        InputRate input_rate_;
        std::uint32_t stride_;
    };

    inline constexpr VertexBinding vertex_binding_v(std::initializer_list<VertexAttributeDesc> attributes)
    {
        return {attributes, InputRate::Vertex};
    }

    inline constexpr VertexBinding vertex_binding_i(std::initializer_list<VertexAttributeDesc> attributes)
    {
        return {attributes, InputRate::Instance};
    }

    struct InputAssemblyDesc {
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
    };

    struct RasterizationDesc {
        FillMode fill_mode = FillMode::Solid;
        CullMode cull_mode = CullMode::Back;
        FrontFace front_face = FrontFace::ClockWise;
    };

    enum class BlendFactor {
        Zero,
        One,
        SrcColor,
        InvertedSrcColor,
        DstColor,
        InvertedDstColor,
        SrcAlpha,
        InvertedSrcAlpha,
        DstAlpha,
        InvertedDstAlpha
    };

    enum class BlendOp {
        Add,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };

    ORION_BITFLAG(ColorComponentFlags, std::uint8_t){
        R = 0x1u,
        G = 0x2u,
        B = 0x4u,
        A = 0x8u,
        All = R | G | B | A};

    struct BlendAttachmentDesc {
        bool enable_blend;
        BlendFactor src_blend;
        BlendFactor dst_blend;
        BlendOp blend_op;
        BlendFactor src_blend_alpha;
        BlendFactor dst_blend_alpha;
        BlendOp blend_op_alpha;
        ColorComponentFlags color_component_flags;
    };

    enum class LogicOp {
        NoOp,
        Clear,
        And,
        AndReverse,
        AndInverted,
        Nand,
        Or,
        OrReverse,
        OrInverted,
        Copy,
        CopyInverted,
        Xor,
        Nor,
        Equivalent,
        Set
    };

    struct ColorBlendDesc {
        bool enable_logic_op = false;
        LogicOp logic_op = LogicOp::NoOp;
        std::span<const BlendAttachmentDesc> attachments;
        std::array<float, 4> blend_constants = {1.f, 1.f, 1.f, 1.f};
    };

    struct GraphicsPipelineDesc {
        std::span<const ShaderStageDesc> shaders = {};
        std::span<const VertexBinding> vertex_bindings = {};
        PipelineLayoutHandle pipeline_layout = PipelineLayoutHandle::invalid();
        InputAssemblyDesc input_assembly = {};
        RasterizationDesc rasterization = {};
        ColorBlendDesc color_blend = {};
        RenderPassHandle render_pass = RenderPassHandle::invalid();
    };

    struct ShaderModuleDesc {
        std::span<const std::byte> byte_code;
    };

    inline constexpr auto default_swapchain_image_count = 2;
    inline constexpr auto default_swapchain_format = Format::B8G8R8A8_Srgb;

    struct SwapchainDesc {
        std::uint32_t image_count = default_swapchain_image_count;
        Format image_format = default_swapchain_format;
        Vector2_u image_size = {};
        ImageUsageFlags image_usage = ImageUsageFlags::ColorAttachment;
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

    struct FenceDesc {
        bool start_finished;
    };

    enum class PipelineBindPoint {
        Graphics,
        Compute
    };

    struct RenderPassDesc {
        std::span<const AttachmentDesc> color_attachments;
        std::span<const AttachmentDesc> input_attachments;
        PipelineBindPoint bind_point;

        [[nodiscard]] auto attachment_count() const noexcept
        {
            return color_attachments.size() + input_attachments.size();
        }
    };

    struct Rect2D {
        Vector2_i offset;
        Vector2_u size;
    };

    struct CommandAllocatorDesc {
        CommandQueueType queue_type;
        bool reset_command_buffer;
    };

    // Forward declare
    class CommandList;

    struct SubmitDesc {
        CommandQueueType queue_type;
        std::span<const SemaphoreHandle> wait_semaphores;
        std::span<const CommandList* const> command_lists;
        std::span<const SemaphoreHandle> signal_semaphores;
    };

    const char* format_as(PhysicalDeviceType type) noexcept;
    const char* format_as(Format format) noexcept;
    std::string format_as(ShaderStageFlags shader_stages);
} // namespace orion
