#include "orion-renderer/effect.h"

#include "orion-assets/config.h"

#include "orion-renderapi/defs.h"
#include "orion-renderapi/render_device.h"
#include "orion-renderapi/shader_reflection.h"

#define JSON_DISABLE_ENUM_SERIALIZATION 1
#include <nlohmann/json.hpp>

#include <algorithm>
#include <array>
#include <string>
#include <utility>
#include <vector>

#ifndef ORION_EFFECT_LOG_LEVEL
    #define ORION_EFFECT_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif
#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    NLOHMANN_JSON_SERIALIZE_ENUM(
        PrimitiveTopology,
        {
            {PrimitiveTopology::TriangleList, "TriangleList"},
        });

    NLOHMANN_JSON_SERIALIZE_ENUM(
        FillMode,
        {
            {FillMode::Solid, "solid"},
            {FillMode::Wireframe, "wireframe"},
        });

    NLOHMANN_JSON_SERIALIZE_ENUM(
        CullMode,
        {
            {CullMode::Back, "back"},
            {CullMode::Front, "front"},
            {CullMode::FrontAndBack, "both"},
        });

    NLOHMANN_JSON_SERIALIZE_ENUM(
        FrontFace,
        {
            {FrontFace::CounterClockWise, "CounterClockWise"},
            {FrontFace::ClockWise, "ClockWise"},
        });

    NLOHMANN_JSON_SERIALIZE_ENUM(
        BlendFactor,
        {
            {BlendFactor::Zero, "Zero"},
            {BlendFactor::One, "One"},
            {BlendFactor::SrcColor, "SrcColor"},
            {BlendFactor::InvertedSrcColor, "InvertedSrcColor"},
            {BlendFactor::DstColor, "DstColor"},
            {BlendFactor::InvertedDstColor, "InvertedDstColor"},
            {BlendFactor::SrcAlpha, "SrcAlpha"},
            {BlendFactor::InvertedSrcColor, "InvertedSrcAlpha"},
            {BlendFactor::DstAlpha, "DstAlpha"},
            {BlendFactor::InvertedDstAlpha, "InvertedDstAlpha"},
        });

    NLOHMANN_JSON_SERIALIZE_ENUM(
        BlendOp,
        {
            {BlendOp::Add, "Add"},
            {BlendOp::Subtract, "Subtract"},
            {BlendOp::ReverseSubtract, "ReverseSubtract"},
            {BlendOp::Min, "Min"},
            {BlendOp::Max, "Max"},
        });

    NLOHMANN_JSON_SERIALIZE_ENUM(
        LogicOp,
        {
            {LogicOp::NoOp, "NoOp"},
            {LogicOp::Clear, "Clear"},
            {LogicOp::And, "And"},
            {LogicOp::AndReverse, "AndReverse"},
            {LogicOp::AndInverted, "AndInverted"},
            {LogicOp::Nand, "Nand"},
            {LogicOp::Or, "Or"},
            {LogicOp::OrReverse, "OrReverse"},
            {LogicOp::OrInverted, "OrInverted"},
            {LogicOp::Copy, "Copy"},
            {LogicOp::CopyInverted, "CopyInverted"},
            {LogicOp::Xor, "Xor"},
            {LogicOp::Nor, "Nor"},
            {LogicOp::Equivalent, "Equivalent"},
            {LogicOp::Set, "Set"},
        });

    NLOHMANN_JSON_SERIALIZE_ENUM(
        Format,
        {
            {Format::Undefined, "Undefined"},
            {Format::R8_Unorm, "R8_Unorm"},
            {Format::B8G8R8A8_Srgb, "B8G8R8A8_Srgb"},
            {Format::R8G8B8A8_Unorm, "R8G8B8A8_Unorm"},
            {Format::R32_Uint, "R32_Uint"},
            {Format::R32_Int, "R32_Int"},
            {Format::R32_Float, "R32_Float"},
            {Format::R32G32_Uint, "R32G32_Uint"},
            {Format::R32G32_Int, "R32G32_Int"},
            {Format::R32G32_Float, "R32G32_Float"},
            {Format::R32G32B32_Uint, "R32G32B32_Uint"},
            {Format::R32G32B32_Int, "R32G32B32_Int"},
            {Format::R32G32B32_Float, "R32G32B32_Float"},
            {Format::R32G32B32A32_Uint, "R32G32B32A32_Uint"},
            {Format::R32G32B32A32_Int, "R32G32B32A32_Int"},
            {Format::R32G32B32A32_Float, "R32G32B32A32_Float"},
        });

    namespace
    {
        auto* logger()
        {
            static const auto logger = create_logger("orion-effect", ORION_EFFECT_LOG_LEVEL);
            return logger.get();
        }

        struct EffectShaders {
            std::string vertex;
            std::string pixel;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EffectShaders, vertex, pixel)

        struct EffectInputAssembly {
            PrimitiveTopology topology;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EffectInputAssembly, topology);

        struct EffectRasterization {
            FillMode fillMode;
            CullMode cullMode;
            FrontFace frontFace;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EffectRasterization, fillMode, cullMode, frontFace);

        struct EffectBlendAttachment {
            bool blendEnable;
            BlendFactor srcBlend;
            BlendFactor dstBlend;
            BlendOp blendOp;
            BlendFactor srcBlendAlpha;
            BlendFactor dstBlendAlpha;
            BlendOp blendOpAlpha;
            std::string colorComponentFlags;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EffectBlendAttachment, blendEnable, srcBlend, dstBlend, blendOp, srcBlendAlpha, dstBlendAlpha, blendOpAlpha, colorComponentFlags);

        struct EffectColorBlend {
            bool logicOpEnable;
            LogicOp logicOp;
            std::vector<EffectBlendAttachment> attachments;
            std::array<float, 4> blendConstants;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EffectColorBlend, logicOpEnable, logicOp, attachments, blendConstants);

        struct EffectRenderTarget {
            Format format;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EffectRenderTarget, format);

        struct EffectPass {
            EffectShaders shaders;
            EffectInputAssembly inputAssembly;
            EffectRasterization rasterization;
            EffectColorBlend colorBlend;
            std::vector<EffectRenderTarget> renderTargets;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EffectPass, shaders, inputAssembly, rasterization, colorBlend, renderTargets);

        struct EffectJSON {
            int version;
            std::vector<EffectPass> passes;
        };
        NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(EffectJSON, version, passes);
    } // namespace

    Effect::Effect(UniqueRenderPass render_pass, std::vector<UniqueDescriptorLayout> descriptor_layouts, UniquePipelineLayout pipeline_layout, UniquePipeline pipeline)
        : render_pass_(std::move(render_pass))
        , descriptor_layouts_(std::move(descriptor_layouts))
        , pipeline_layout_(std::move(pipeline_layout))
        , pipeline_(std::move(pipeline))
    {
    }

    EffectCompiler::EffectCompiler(RenderDevice* device, ShaderReflector* shader_reflector)
        : device_(device)
        , shader_reflector_(shader_reflector)
    {
    }

    Effect EffectCompiler::compile_file(const File& file, const EffectCompileDesc& desc)
    {
        SPDLOG_LOGGER_TRACE(logger(), "Creating effect...");
        const auto file_contents = file.read_all_str();

        using json = nlohmann::json;
        const auto effect_json = json::parse(file_contents);

        const auto effect = effect_json.get<EffectJSON>();
        SPDLOG_LOGGER_TRACE(logger(), "effect file version: {}", effect.version);
        SPDLOG_LOGGER_TRACE(logger(), "- passes {}", effect.passes.size());
        if (effect.passes.empty()) {
            throw std::runtime_error{"no passes defined in effect"};
        }

        const auto pass = effect.passes[0];

        const auto vs_binary = binary_input_file(desc.shader_base_path / pass.shaders.vertex).read_all();
        const auto vs_module = device_->make_unique<ShaderModuleHandle_tag>(ShaderModuleDesc{vs_binary});
        const auto vs_reflection = shader_reflector_->reflect(vs_binary).value();

        const auto ps_binary = binary_input_file(desc.shader_base_path / pass.shaders.pixel).read_all();
        const auto ps_module = device_->make_unique<ShaderModuleHandle_tag>(ShaderModuleDesc{ps_binary});
        const auto ps_reflection = shader_reflector_->reflect(ps_binary).value();

        const auto shader_stages = std::array{
            ShaderStageDesc{
                .module = vs_module.get(),
                .stage = ShaderStageFlags::Vertex,
                .entry_point = ORION_VS_ENTRY,
            },
            ShaderStageDesc{
                .module = ps_module.get(),
                .stage = ShaderStageFlags::Pixel,
                .entry_point = ORION_PS_ENTRY,
            },
        };

        std::vector<VertexAttributeDesc> vertex_attrs(vs_reflection.input_variables.size());
        std::ranges::transform(vs_reflection.input_variables, vertex_attrs.begin(), [](const auto& var) {
            return VertexAttributeDesc{
                .name = var.name.c_str(),
                .format = var.format,
            };
        });

        const auto vertex_bindings = std::array{
            VertexBinding{vertex_attrs, InputRate::Vertex},
        };

        // TODO: Get descriptors from shader reflection
        auto pipeline_layout = device_->make_unique<PipelineLayoutHandle_tag>(PipelineLayoutDesc{
            .descriptors = {},
            .push_constants = {},
        });

        const auto input_assembly = InputAssemblyDesc{
            .topology = pass.inputAssembly.topology,
        };

        const auto rasterization = RasterizationDesc{
            .fill_mode = pass.rasterization.fillMode,
            .cull_mode = pass.rasterization.cullMode,
            .front_face = pass.rasterization.frontFace,
        };

        std::vector<BlendAttachmentDesc> blend_attachments(pass.colorBlend.attachments.size());
        std::ranges::transform(pass.colorBlend.attachments, blend_attachments.begin(), [](const EffectBlendAttachment& attachment) {
            // TODO: Parse color component flags
            const auto color_component = ColorComponentFlags::All;
            return BlendAttachmentDesc{
                attachment.blendEnable,
                attachment.srcBlend,
                attachment.dstBlend,
                attachment.blendOp,
                attachment.srcBlendAlpha,
                attachment.dstBlendAlpha,
                attachment.blendOpAlpha,
                color_component,
            };
        });

        const auto color_blend = ColorBlendDesc{
            .enable_logic_op = pass.colorBlend.logicOpEnable,
            .logic_op = pass.colorBlend.logicOp,
            .attachments = blend_attachments,
            .blend_constants = pass.colorBlend.blendConstants,
        };

        std::vector<AttachmentDesc> color_attachments(pass.renderTargets.size());
        std::ranges::transform(pass.renderTargets, color_attachments.begin(), [](const auto& render_target) {
            return AttachmentDesc{.format = render_target.format};
        });
        auto render_pass = device_->make_unique<RenderPassHandle_tag>(RenderPassDesc{
            .color_attachments = color_attachments,
            .bind_point = PipelineBindPoint::Graphics,
        });

        auto pipeline = device_->make_unique<PipelineHandle_tag>(GraphicsPipelineDesc{
            shader_stages,
            vertex_bindings,
            pipeline_layout.get(),
            input_assembly,
            rasterization,
            color_blend,
            render_pass.get(),
        });

        return Effect{std::move(render_pass), {}, std::move(pipeline_layout), std::move(pipeline)};
    }
} // namespace orion