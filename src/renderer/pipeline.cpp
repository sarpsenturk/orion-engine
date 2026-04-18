#include "orion/renderer/pipeline.hpp"

#include "orion/debug.hpp"

#include <fstream>

namespace orion
{
    static VkPipelineColorBlendAttachmentState to_vk_blend_attachment(BlendMode blend_mode)
    {
        static constexpr auto color_write_all = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        switch (blend_mode) {
            case BlendMode::Opaque:
                return {
                    .blendEnable = VK_FALSE,
                    .colorWriteMask = color_write_all,
                };
        }
        unreachable();
    }

    PipelineBuilder::PipelineBuilder(VkPipelineLayout layout)
        : vs_info_({.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO})
        , fs_info_({.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO})
        , shader_stages_({
              VkPipelineShaderStageCreateInfo{
                  .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                  .pNext = &vs_info_,
                  .stage = VK_SHADER_STAGE_VERTEX_BIT,
                  .pName = "main",
              },
              VkPipelineShaderStageCreateInfo{
                  .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                  .pNext = &fs_info_,
                  .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                  .pName = "main",
              },
          })
        , rendering_state_{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
              .viewMask = 0,
              .colorAttachmentCount = 0,
              .pColorAttachmentFormats = nullptr,
              .depthAttachmentFormat = VK_FORMAT_UNDEFINED,
              .stencilAttachmentFormat = VK_FORMAT_UNDEFINED,
          }
        , vertex_input_state_{.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO}
        , input_assembly_state_{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
              .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
              .primitiveRestartEnable = VK_FALSE,
          }
        , viewport_state_{.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO}
        , rasterization_state_{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
              .depthClampEnable = VK_FALSE,
              .rasterizerDiscardEnable = VK_FALSE,
              .polygonMode = VK_POLYGON_MODE_FILL,
              .cullMode = VK_CULL_MODE_BACK_BIT,
              .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
              .depthBiasEnable = VK_FALSE,
              .depthBiasConstantFactor = 0.0f,
              .depthBiasClamp = 0.0f,
              .depthBiasSlopeFactor = 1.0f,
              .lineWidth = 1.0f,
          }
        , multisample_state_{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
              .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
              .sampleShadingEnable = VK_FALSE,
              .minSampleShading = 0.0f,
              .pSampleMask = nullptr,
              .alphaToCoverageEnable = VK_FALSE,
              .alphaToOneEnable = VK_FALSE,
          }
        , depth_stencil_state_{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
              .depthTestEnable = VK_FALSE,
              .depthWriteEnable = VK_FALSE,
              .depthCompareOp = VK_COMPARE_OP_GREATER,
              .depthBoundsTestEnable = VK_FALSE,
              .stencilTestEnable = VK_FALSE,
              .front = {},
              .back = {},
              .minDepthBounds = 0.0f,
              .maxDepthBounds = 1.0f,
          }
        , color_blend_state_{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
              .logicOpEnable = VK_FALSE,
          }
        , dynamic_state_{
              .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
              .dynamicStateCount = static_cast<std::uint32_t>(dynamic_states.size()),
              .pDynamicStates = dynamic_states.data(),
          }
        , layout_(layout)
    {
    }

    void PipelineBuilder::set_vertex_shader(const ShaderPath& shader)
    {
        auto file = std::ifstream{shader, std::ios::binary};
        ORION_ASSERT(file.good());
        const auto length = std::filesystem::file_size(shader);
        vs_code_.resize(length / sizeof(std::uint32_t));
        file.read(vs_code_.data(), static_cast<std::streamsize>(length));
        vs_info_.codeSize = length;
        vs_info_.pCode = reinterpret_cast<std::uint32_t*>(vs_code_.data());
    }

    void PipelineBuilder::set_fragment_shader(const ShaderPath& shader)
    {
        auto file = std::ifstream{shader, std::ios::binary};
        ORION_ASSERT(file.good());
        const auto length = std::filesystem::file_size(shader);
        vs_code_.resize(length / sizeof(std::uint32_t));
        file.read(fs_code_.data(), static_cast<std::streamsize>(length));
        fs_info_.codeSize = length;
        fs_info_.pCode = reinterpret_cast<std::uint32_t*>(fs_code_.data());
    }

    void PipelineBuilder::add_vertex_binding(std::uint32_t binding, std::uint32_t stride, VkVertexInputRate input_rate)
    {
        vertex_bindings_.emplace_back(binding, stride, input_rate);
        vertex_input_state_.vertexBindingDescriptionCount = static_cast<std::uint32_t>(vertex_bindings_.size());
        vertex_input_state_.pVertexBindingDescriptions = vertex_bindings_.data();
    }

    void PipelineBuilder::add_vertex_attribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset)
    {
        vertex_attributes_.emplace_back(location, binding, format, offset);
        vertex_input_state_.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(vertex_attributes_.size());
        vertex_input_state_.pVertexAttributeDescriptions = vertex_attributes_.data();
    }

    void PipelineBuilder::set_viewport_count(std::uint32_t viewport_count)
    {
        viewport_state_.viewportCount = viewport_count;
    }

    void PipelineBuilder::set_scissor_count(std::uint32_t scissor_count)
    {
        viewport_state_.scissorCount = scissor_count;
    }

    void PipelineBuilder::set_polygon_mode(VkPolygonMode polygon_mode)
    {
        rasterization_state_.polygonMode = polygon_mode;
    }

    void PipelineBuilder::set_cull_mode(VkCullModeFlags cull_mode)
    {
        rasterization_state_.cullMode = cull_mode;
    }

    void PipelineBuilder::set_front_face(VkFrontFace front_face)
    {
        rasterization_state_.frontFace = front_face;
    }

    void PipelineBuilder::add_color_attachment(VkFormat format, BlendMode blend_mode)
    {
        color_attachments_.push_back(format);
        rendering_state_.colorAttachmentCount = static_cast<std::uint32_t>(color_attachments_.size());
        rendering_state_.pColorAttachmentFormats = color_attachments_.data();

        blend_attachments_.push_back(to_vk_blend_attachment(blend_mode));
        color_blend_state_.attachmentCount = static_cast<std::uint32_t>(blend_attachments_.size());
        color_blend_state_.pAttachments = blend_attachments_.data();
    }

    VkGraphicsPipelineCreateInfo PipelineBuilder::build() const
    {
        return {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .pNext = &rendering_state_,
            .stageCount = static_cast<std::uint32_t>(shader_stages_.size()),
            .pStages = shader_stages_.data(),
            .pVertexInputState = &vertex_input_state_,
            .pInputAssemblyState = &input_assembly_state_,
            .pTessellationState = nullptr,
            .pViewportState = &viewport_state_,
            .pRasterizationState = &rasterization_state_,
            .pMultisampleState = &multisample_state_,
            .pDepthStencilState = &depth_stencil_state_,
            .pColorBlendState = &color_blend_state_,
            .pDynamicState = &dynamic_state_,
            .layout = layout_,
            .renderPass = VK_NULL_HANDLE,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = 0,
        };
    }
} // namespace orion
