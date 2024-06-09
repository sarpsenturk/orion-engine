#pragma once

#include "orion-renderapi/defs.h"
#include "orion-renderapi/device_resource.h"

#include "orion-core/filesystem.h"

#include <array>
#include <span>
#include <vector>

namespace orion
{
    class ShaderEffect
    {
    public:
        ShaderEffect(UniqueShaderModule vertex_shader,
                     UniqueShaderModule pixel_shader,
                     std::vector<VertexAttributeDesc> vertex_attributes,
                     std::array<UniqueDescriptorLayout, 4> descriptor_layouts,
                     UniquePipelineLayout pipeline_layout);

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

    class ShaderPass
    {
    public:
        ShaderPass(const ShaderEffect* effect, UniqueRenderPass render_pass, UniquePipeline pipeline);

        [[nodiscard]] const ShaderEffect* effect() const { return effect_; }
        [[nodiscard]] PipelineHandle pipeline() const { return pipeline_.get(); }

    private:
        const ShaderEffect* effect_;
        UniqueRenderPass render_pass_;
        UniquePipeline pipeline_;
    };

    // Forward declare
    class RenderDevice;

    ShaderEffect create_shader_effect(RenderDevice* device, const FilePath& vs_path, const FilePath& ps_path);

    enum class ShaderPassBlend {
        Disable,
        Add,
        Transparent,
    };

    struct ShaderPassDesc {
        RasterizationDesc rasterization = {};
        ShaderPassBlend blend = ShaderPassBlend::Disable;
    };

    ShaderPass create_shader_pass(RenderDevice* device, const ShaderEffect* effect, const ShaderPassDesc& desc);
} // namespace orion
