#pragma once

#include "orion-renderer/types.h"

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

    class Pipeline
    {
    public:
        Pipeline() = default;
        Pipeline(const ShaderEffect* effect, UniquePipeline pipeline);

        void set_effect(const ShaderEffect* effect) { effect_ = effect; }

        void set_cull(CullMode cull_mode) { rasterization_.cull_mode = cull_mode; }
        void cull_none() { set_cull(CullMode::None); }
        void cull_back() { set_cull(CullMode::Back); }
        void cull_front() { set_cull(CullMode::Front); }

        void set_front_face(FrontFace front_face) { rasterization_.front_face = front_face; }
        void front_clockwise() { set_front_face(FrontFace::ClockWise); }
        void front_counterclockwise() { set_front_face(FrontFace::CounterClockWise); }

        void set_blend(BlendMode blend_mode) { blend_mode_ = blend_mode; }
        void blend_disable() { set_blend(BlendMode::Disabled); }
        void blend_add() { set_blend(BlendMode::Add); }
        void blend_transparent() { set_blend(BlendMode::Transparent); }

        void create(RenderDevice* device);

        [[nodiscard]] const ShaderEffect* effect() const { return effect_; }
        [[nodiscard]] PipelineHandle pipeline() const { return pipeline_.get(); }

    private:
        const ShaderEffect* effect_ = nullptr;
        RasterizationDesc rasterization_ = {};
        BlendMode blend_mode_ = BlendMode::Disabled;

        UniquePipeline pipeline_;
    };
} // namespace orion
