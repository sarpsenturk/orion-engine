#pragma once

#include "handles.h"
#include "render_target.h"
#include "shader.h"
#include "types.h"

#include <initializer_list> // std::initializer_list
#include <span>             // std::span
#include <vector>           // std::vector

namespace orion
{
    struct ShaderStageDesc {
        ShaderModule module;
        ShaderType type;
        const char* entry_point = "main";
    };

    struct VertexAttributeDesc {
        const char* name;
        Format format;
        std::uint32_t offset = UINT32_MAX;
    };

    class VertexBinding
    {
    public:
        VertexBinding(std::initializer_list<VertexAttributeDesc> attributes, InputRate input_rate);

        [[nodiscard]] auto& attributes() const noexcept { return attributes_; }
        [[nodiscard]] auto input_rate() const noexcept { return input_rate_; }
        [[nodiscard]] auto stride() const noexcept { return stride_; }

    private:
        std::uint32_t calculate_stride_and_offsets() noexcept;

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
        InputAssemblyDesc input_assembly = {};
        RasterizationDesc rasterization = {};
        RenderTarget render_target;
    };

    class GraphicsPipeline
    {
    public:
        GraphicsPipeline() = default;
        GraphicsPipeline(GraphicsPipelineHandleRef handle);

    private:
        GraphicsPipelineHandleRef handle_;
    };
} // namespace orion
