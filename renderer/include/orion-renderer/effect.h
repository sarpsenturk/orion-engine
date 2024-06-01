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
        Effect(UniqueRenderPass render_pass, UniquePipeline pipeline);

        [[nodiscard]] PipelineHandle pipeline() const { return pipeline_.get(); }

    private:
        UniqueRenderPass render_pass_;
        UniquePipeline pipeline_;
    };

    class EffectCompiler
    {
    public:
        EffectCompiler(RenderDevice* device, ShaderReflector* shader_reflector, PipelineLayoutHandle pipeline_layout);

        Effect compile_file(const File& file);

    private:
        RenderDevice* device_;
        ShaderReflector* shader_reflector_;
        PipelineLayoutHandle pipeline_layout_;
    };
} // namespace orion
