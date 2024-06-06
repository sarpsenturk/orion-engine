#pragma once

#include "orion-renderapi/defs.h"
#include "orion-renderapi/device_resource.h"

#include "orion-core/filesystem.h"

namespace orion
{
    class ShaderEffect
    {
    public:
        ShaderEffect(UniqueShaderModule vertex_shader,
                     UniqueShaderModule pixel_shader,
                     std::array<UniqueDescriptorLayout, 4> descriptor_layouts,
                     UniquePipelineLayout pipeline_layout);

        [[nodiscard]] ShaderModuleHandle vertex_shader() const { return vertex_shader_.get(); }
        [[nodiscard]] ShaderModuleHandle pixel_shader() const { return pixel_shader_.get(); }

        [[nodiscard]] std::array<ShaderStageDesc, 2> shader_stages() const;
        [[nodiscard]] PipelineLayoutHandle pipeline_layout() const { return pipeline_layout_.get(); }

    private:
        UniqueShaderModule vertex_shader_;
        UniqueShaderModule pixel_shader_;
        std::array<UniqueDescriptorLayout, 4> descriptor_layouts_;
        UniquePipelineLayout pipeline_layout_;
    };

    // Forward declare
    class RenderDevice;

    ShaderEffect create_shader_effect(RenderDevice* device, const FilePath& vs_path, const FilePath& ps_path);
} // namespace orion
