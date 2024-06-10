#pragma once

#include "orion-renderapi/device_resource.h"
#include "orion-renderapi/pipeline.h"
#include "orion-renderapi/shader.h"

#include "orion-core/filesystem.h"

#include <array>
#include <span>
#include <vector>

namespace orion
{
    // Forward declare
    class RenderDevice;

    class ShaderEffect
    {
    public:
        ShaderEffect(UniqueShaderModule vertex_shader,
                     UniqueShaderModule pixel_shader,
                     std::vector<VertexAttributeDesc> vertex_attributes,
                     std::array<UniqueDescriptorLayout, 4> descriptor_layouts,
                     UniquePipelineLayout pipeline_layout);

        static ShaderEffect create(RenderDevice* device, const FilePath& vs_path, const FilePath& ps_path);

        [[nodiscard]] ShaderModuleHandle vertex_shader() const { return vertex_shader_.get(); }
        [[nodiscard]] ShaderModuleHandle pixel_shader() const { return pixel_shader_.get(); }
        [[nodiscard]] std::array<ShaderStageDesc, 2> shader_stages() const;
        [[nodiscard]] auto vertex_attributes() const { return std::span{vertex_attributes_}; }
        [[nodiscard]] PipelineLayoutHandle pipeline_layout() const { return pipeline_layout_.get(); }
        [[nodiscard]] DescriptorLayoutHandle descriptor_layout(std::uint32_t index) const { return descriptor_layouts_[index].get(); }

    private:
        UniqueShaderModule vertex_shader_;
        UniqueShaderModule pixel_shader_;
        std::vector<VertexAttributeDesc> vertex_attributes_;
        std::array<UniqueDescriptorLayout, 4> descriptor_layouts_;
        UniquePipelineLayout pipeline_layout_;
    };

    enum class ShaderPassBlend {
        Disable,
        Add,
        Transparent,
    };

    struct ShaderPassDesc {
        RasterizationDesc rasterization = {};
        ShaderPassBlend blend = ShaderPassBlend::Disable;
    };

    class ShaderPass
    {
    public:
        ShaderPass(const ShaderEffect* effect, UniqueRenderPass render_pass, UniquePipeline pipeline);

        static ShaderPass create(RenderDevice* device, const ShaderEffect* effect, const ShaderPassDesc& desc);

        [[nodiscard]] const ShaderEffect* effect() const { return effect_; }
        [[nodiscard]] PipelineHandle pipeline() const { return pipeline_.get(); }

    private:
        const ShaderEffect* effect_;
        UniqueRenderPass render_pass_;
        UniquePipeline pipeline_;
    };
} // namespace orion
