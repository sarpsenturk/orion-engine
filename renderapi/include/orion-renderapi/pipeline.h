#pragma once

#include "orion-renderapi/blend.h"
#include "orion-renderapi/format.h"
#include "orion-renderapi/handles.h"
#include "orion-renderapi/shader.h"

#include "orion-math/vector/vector2.h"

namespace orion
{
    struct ShaderStageDesc {
        ShaderModuleHandle module = ShaderModuleHandle::invalid();
        ShaderStageFlags stage = {};
        const char* entry_point = "main";
    };

    struct VertexAttributeDesc {
        std::string name;
        Format format;
    };

    std::uint32_t vertex_input_stride(std::span<const VertexAttributeDesc> attributes);

    struct PushConstantDesc {
        std::uint32_t size;
        ShaderStageFlags shader_stages;
    };

    struct PipelineLayoutDesc {
        std::span<const DescriptorLayoutHandle> descriptors;
        std::span<const PushConstantDesc> push_constants;
    };

    enum class PrimitiveTopology {
        TriangleList
    };

    struct InputAssemblyDesc {
        PrimitiveTopology topology = PrimitiveTopology::TriangleList;
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

    struct RasterizationDesc {
        FillMode fill_mode = FillMode::Solid;
        CullMode cull_mode = CullMode::Back;
        FrontFace front_face = FrontFace::ClockWise;
    };

    struct GraphicsPipelineDesc {
        std::span<const ShaderStageDesc> shaders = {};
        std::span<const VertexAttributeDesc> vertex_attributes = {};
        PipelineLayoutHandle pipeline_layout = PipelineLayoutHandle::invalid();
        InputAssemblyDesc input_assembly = {};
        RasterizationDesc rasterization = {};
        ColorBlendDesc color_blend = {};
        std::span<const Format> render_targets;
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

} // namespace orion
