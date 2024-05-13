#pragma once

#include "orion-core/filesystem.h"

#include "orion-renderapi/device_resource.h"

#include <string_view>
#include <vector>

namespace orion
{
    class RenderDevice;
    class ShaderReflector;

    class Effect
    {
    public:
        Effect(UniqueRenderPass render_pass, std::vector<UniqueDescriptorLayout> descriptor_layouts, UniquePipelineLayout pipeline_layout, UniquePipeline pipeline);

        [[nodiscard]] auto& descriptor_layouts() const { return descriptor_layouts_; }
        [[nodiscard]] PipelineLayoutHandle pipeline_layout() const { return pipeline_layout_.get(); }
        [[nodiscard]] PipelineHandle pipeline() const { return pipeline_.get(); }

    private:
        UniqueRenderPass render_pass_;
        std::vector<UniqueDescriptorLayout> descriptor_layouts_;
        UniquePipelineLayout pipeline_layout_;
        UniquePipeline pipeline_;
    };

    struct EffectCompileDesc {
        FilePath shader_base_path;
    };

    class EffectCompiler
    {
    public:
        EffectCompiler(RenderDevice* device, ShaderReflector* shader_reflector);

        Effect compile_file(const File& file, const EffectCompileDesc& desc);

    private:
        RenderDevice* device_;
        ShaderReflector* shader_reflector_;
    };
} // namespace orion
