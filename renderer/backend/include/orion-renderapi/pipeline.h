#pragma once

#include "handles.h"
#include "shader.h"
#include "types.h"

#include <initializer_list> // std::initializer_list
#include <span>             // std::span
#include <vector>           // std::vector

namespace orion
{
    struct ShaderStageDesc {
        ShaderModuleHandle module;
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
        InputAssemblyDesc input_assembly = {};
        RasterizationDesc rasterization = {};
        RenderTargetHandle render_target;
    };
} // namespace orion
