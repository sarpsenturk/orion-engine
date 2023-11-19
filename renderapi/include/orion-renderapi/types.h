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

namespace orion
{
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

    using physical_device_index_t = std::int32_t;
    inline constexpr auto invalid_physical_device_index = -1;

    struct PhysicalDeviceDesc {
        physical_device_index_t index;
        PhysicalDeviceType type;
        std::string name;
    };

    using pfnSelectPhysicalDevice = physical_device_index_t (*)(std::span<const PhysicalDeviceDesc>);

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
            case Format::R8_Unorm:
                return sizeof(std::uint8_t);
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
            case Format::R8_Unorm:
                return "R8_Unorm";
            case Format::R8G8B8A8_Unorm:
                return "R8G8B8A8_Unorm";
        }
        return "Unknown format";
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

    enum class ShaderStageFlags : std::uint8_t {
        Vertex = 0x1,
        Pixel = 0x2
    };

    template<>
    struct enum_bitwise_enabled<ShaderStageFlags> : std::true_type {
    };

    std::string format_as(ShaderStageFlags shader_stages);

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

    enum class GPUBufferUsageFlags : std::uint8_t {
        VertexBuffer = 0x1,
        IndexBuffer = 0x2,
        ConstantBuffer = 0x4,
        TransferSrc = 0x8,
        TransferDst = 0x10,
        Transfer = TransferSrc | TransferDst
    };
    template<>
    struct enum_bitwise_enabled<GPUBufferUsageFlags> : std::true_type {
    };

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
    };

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

    struct BufferBindingDesc {
        GPUBufferHandle buffer_handle = GPUBufferHandle::invalid();
        std::size_t size;
        std::size_t offset;
    };

    struct DescriptorBinding {
        std::uint32_t binding;
        BindingType binding_type;
        BufferBindingDesc buffer;

        [[nodiscard]] bool is_buffer() const noexcept { return buffer.buffer_handle.is_valid(); }
    };

    [[nodiscard]] bool is_buffer_binding(const DescriptorBinding& binding);

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

    enum class ImageUsageFlags : std::uint8_t {
        TransferSrc = 0x1,
        TransferDst = 0x2,
        ColorAttachment = 0x4,
        DepthStencilAttachment = 0x8,
        InputAttachment = 0x10,
        SampledImage = 0x20,
        Transfer = TransferSrc | TransferDst
    };
    template<>
    struct enum_bitwise_enabled<ImageUsageFlags> : std::true_type {
    };

    enum class ImageViewType {
        View1D,
        View2D,
        View3D,
        ViewCube,
        View1DArray,
        View2DArray,
        ViewCubeArray,
    };

    enum class ResourceAccessFlags : std::uint8_t {
        ColorAttachmentWrite = 0x1,
        TransferRead = 0x2,
        TransferWrite = 0x4,
        MemoryRead = 0x8,
        MemoryWrite = 0x10,
        ShaderRead = 0x20,
        ShaderWrite = 0x40
    };
    template<>
    struct enum_bitwise_enabled<ResourceAccessFlags> : std::true_type {
    };

    enum class PipelineStageFlags : std::uint8_t {
        TopOfPipe = 0x1,
        ColorAttachmentOutput = 0x2,
        Transfer = 0x4,
        BottomOfPipe = 0x8,
        PixelShader = 0x10
    };
    template<>
    struct enum_bitwise_enabled<PipelineStageFlags> : std::true_type {
    };

    enum class IndexType {
        None = 0,
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
        std::span<const AttachmentDesc> input_attachments;

        [[nodiscard]] auto attachment_count() const noexcept
        {
            return color_attachments.size() + input_attachments.size();
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
        ShaderModuleHandle module = ShaderModuleHandle::invalid();
        ShaderStageFlags stage = {};
        const char* entry_point = "main";
    };

    struct VertexAttributeDesc {
        const char* name;
        Format format;
        std::uint32_t offset;
    };

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

        static_vector<VertexAttributeDesc, max_attribute_count> attributes_;
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

    enum class ColorComponentFlags : std::uint8_t {
        R = 0x1u,
        G = 0x2u,
        B = 0x4u,
        A = 0x8u,
        All = R | G | B | A
    };
    template<>
    struct enum_bitwise_enabled<ColorComponentFlags> : std::true_type {
    };

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
        bool enable_logic_op;
        LogicOp logic_op;
        std::span<const BlendAttachmentDesc> attachments;
        std::array<float, 4> blend_constants;
    };

    struct GraphicsPipelineDesc {
        std::span<const ShaderStageDesc> shaders = {};
        std::span<const VertexBinding> vertex_bindings = {};
        PipelineLayoutHandle pipeline_layout = PipelineLayoutHandle::invalid();
        InputAssemblyDesc input_assembly = {};
        RasterizationDesc rasterization = {};
        ColorBlendDesc color_blend = {};
        AttachmentList attachment_list;
    };

    struct RenderPassDesc {
        AttachmentList attachments;
    };

    struct ShaderModuleDesc {
        std::span<const char> byte_code;
    };

    inline constexpr auto default_swapchain_image_count = 2;
    inline constexpr auto default_swapchain_format = Format::B8G8R8A8_Srgb;

    struct SwapchainDesc {
        class Window* window = nullptr;
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

    struct GPUJobDesc {
        std::span<const GPUJobHandle> dependencies;
        bool start_finished;
    };

    enum class PipelineBindPoint {
        Graphics,
        Compute
    };
} // namespace orion
