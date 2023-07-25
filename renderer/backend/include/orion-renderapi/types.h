#pragma once

#include <orion-math/vector/vector2.h>
#include <orion-utils/bitflag.h>

#include <cstdint>
#include <span>
#include <string>

namespace orion
{
    enum class Format : std::uint32_t {
        Undefined,
        B8G8R8A8_Srgb,
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
        TransferDst
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
    };

    enum class CommandQueueType {
        Graphics,
        Transfer,
        Compute,
        Any
    };

    enum class CommandType {
        BufferCopy,
        BeginFrame,
        EndFrame,
        Draw,
        DrawIndexed,
        BindDescriptorSets,
        PipelineBarrier,
        BlitImage
    };

    enum class DescriptorType : std::uint8_t {
        Unknown,
        ConstantBuffer
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
        InputAttachment
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
        MemoryRead
    };
    using ResourceAccessFlags = Bitflag<ResourceAccess>;

    enum class PipelineStage : std::uint8_t {
        TopOfPipe,
        ColorAttachmentOutput,
        Transfer,
        BottomOfPipe
    };
    using PipelineStageFlags = Bitflag<PipelineStage>;
} // namespace orion
