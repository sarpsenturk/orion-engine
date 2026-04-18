#include "orion/renderer/pipeline.hpp"

#include "orion/config.h"
#include "orion/debug.hpp"
#include "orion/log.hpp"

#include <vulkan/vk_enum_string_helper.h>

#include <fstream>
#include <utility>

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

    void load_shader(const ShaderPath& shader, std::vector<std::uint32_t>& code, VkShaderModuleCreateInfo& shader_info)
    {
        auto file = std::ifstream{shader, std::ios::binary};
        ORION_ASSERT(file.good());
        const auto length = std::filesystem::file_size(shader);
        code.resize(length / sizeof(std::uint32_t));
        file.read(reinterpret_cast<char*>(code.data()), static_cast<std::streamsize>(length));
        shader_info.codeSize = length;
        shader_info.pCode = reinterpret_cast<std::uint32_t*>(code.data());
    }

    PipelineBuilder::PipelineBuilder(VkPipelineLayout layout)
        : shader_stages_({
              VkPipelineShaderStageCreateInfo{
                  .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                  .stage = VK_SHADER_STAGE_VERTEX_BIT,
                  .pName = "main",
              },
              VkPipelineShaderStageCreateInfo{
                  .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                  .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                  .pName = "main",
              },
          })
        , vs_info_({.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO})
        , fs_info_({.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO})
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
        load_shader(std::filesystem::path(ORION_BINARY_DIR) / shader, vs_code_, vs_info_);
    }

    void PipelineBuilder::set_fragment_shader(const ShaderPath& shader)
    {
        load_shader(std::filesystem::path(ORION_BINARY_DIR) / shader, fs_code_, fs_info_);
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

    void PipelineBuilder::set_depth_attachment(VkFormat format)
    {
        rendering_state_.depthAttachmentFormat = format;
    }

    tl::expected<VkPipeline, VkResult> PipelineBuilder::build(VkDevice device)
    {
        // Create shader modules
        if (VkResult err = vkCreateShaderModule(device, &vs_info_, nullptr, &shader_stages_[0].module)) {
            ORION_RENDERER_LOG_ERROR("vkCreateShaderModule() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        }
        if (VkResult err = vkCreateShaderModule(device, &fs_info_, nullptr, &shader_stages_[1].module)) {
            ORION_RENDERER_LOG_ERROR("vkCreateShaderModule() failed: {}", string_VkResult(err));
            vkDestroyShaderModule(device, shader_stages_[0].module, nullptr);
            return tl::unexpected(err);
        }

        // Create pipeline
        const auto pipeline_info = VkGraphicsPipelineCreateInfo{
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
        VkPipeline pipeline = VK_NULL_HANDLE;
        VkResult err = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline);
        vkDestroyShaderModule(device, shader_stages_[1].module, nullptr);
        vkDestroyShaderModule(device, shader_stages_[0].module, nullptr);
        if (err) {
            ORION_RENDERER_LOG_ERROR("vkCreateGraphicsPipelines() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        } else {
            return pipeline;
        }
    }

    PipelineCache::PipelineCache(VkDevice device, VkPipelineLayout pipeline_layout)
        : vk_device_(device)
        , pipeline_layout_(pipeline_layout)
    {
    }

    PipelineCache::PipelineCache(PipelineCache&& other) noexcept
        : vk_device_(other.vk_device_)
        , pipeline_layout_(std::exchange(other.pipeline_layout_, VK_NULL_HANDLE))
        , pipelines_(std::move(other.pipelines_))
    {
    }

    PipelineCache& PipelineCache::operator=(PipelineCache&& other) noexcept
    {
        if (this != &other) {
            for (const auto& [_, pipeline] : pipelines_) {
                vkDestroyPipeline(vk_device_, pipeline, nullptr);
                ORION_RENDERER_LOG_INFO("Destroyed VkPipeline {}", fmt::ptr(pipeline));
            }
            if (pipeline_layout_ != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(vk_device_, pipeline_layout_, nullptr);
                ORION_RENDERER_LOG_INFO("Destroyed VkPipelineLayout {}", fmt::ptr(pipeline_layout_));
            }
            vk_device_ = other.vk_device_;
            pipeline_layout_ = std::exchange(other.pipeline_layout_, VK_NULL_HANDLE);
            pipelines_ = std::move(other.pipelines_);
        }
        return *this;
    }

    PipelineCache::~PipelineCache()
    {
        for (const auto& [_, pipeline] : pipelines_) {
            vkDestroyPipeline(vk_device_, pipeline, nullptr);
            ORION_RENDERER_LOG_INFO("Destroyed VkPipeline {}", fmt::ptr(pipeline));
        }
        if (pipeline_layout_ != VK_NULL_HANDLE) {
            vkDestroyPipelineLayout(vk_device_, pipeline_layout_, nullptr);
            ORION_RENDERER_LOG_INFO("Destroyed VkPipelineLayout {}", fmt::ptr(pipeline_layout_));
        }
    }

    tl::expected<PipelineCache, VkResult> PipelineCache::initialize(VkDevice device)
    {
        // Create fixed pipeline layout
        const auto pipeline_layout_info = VkPipelineLayoutCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .pNext = nullptr,
            .flags = {},
            .setLayoutCount = 0,
            .pSetLayouts = nullptr,
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
        };
        VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
        if (VkResult err = vkCreatePipelineLayout(device, &pipeline_layout_info, nullptr, &pipeline_layout)) {
            ORION_RENDERER_LOG_ERROR("vkCreatePipelineLayout() failed: {}", string_VkResult(err));
            return tl::unexpected(err);
        } else {
            ORION_RENDERER_LOG_INFO("Created VkPipelineLayout {}", fmt::ptr(pipeline_layout));
        }
        return PipelineCache{device, pipeline_layout};
    }

    tl::expected<VkPipeline, VkResult> PipelineCache::build(std::string name, PipelineBuilder& builder)
    {
        auto pipeline = builder.build(vk_device_);
        if (pipeline) {
            ORION_RENDERER_LOG_INFO("Created VkPipeline (graphics) {} ({})", fmt::ptr(*pipeline), name);
            auto [it, inserted] = pipelines_.insert(std::make_pair(std::move(name), *pipeline));
            ORION_ASSERT(inserted);
        }
        return pipeline;
    }

    VkPipeline PipelineCache::get(std::string_view name) const
    {
        if (auto it = pipelines_.find(name); it != pipelines_.end()) {
            return it->second;
        } else {
            return VK_NULL_HANDLE;
        }
    }
} // namespace orion
