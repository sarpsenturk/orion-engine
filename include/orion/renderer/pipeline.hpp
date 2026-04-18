#pragma once

#include <volk.h>

#include <tl/expected.hpp>

#include <array>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace orion
{
    // Path to SPIR-V shader object
    using ShaderPath = std::filesystem::path;

    // Color blend configuration to use
    enum class BlendMode {
        Opaque = 0,
    };

    class PipelineBuilder
    {
    public:
        explicit PipelineBuilder(VkPipelineLayout layout);

        // Shaders

        void set_vertex_shader(const ShaderPath& shader);
        void set_fragment_shader(const ShaderPath& shader);

        // Attachments

        void add_color_attachment(VkFormat format, BlendMode blend_mode = BlendMode::Opaque);
        void set_depth_attachment(VkFormat format);

        // Vertex input state

        void add_vertex_binding(std::uint32_t binding, std::uint32_t stride, VkVertexInputRate input_rate);
        void add_vertex_attribute(uint32_t location, uint32_t binding, VkFormat format, uint32_t offset);

        // Input assembly state

        void set_primitive_topology(VkPrimitiveTopology topology);

        // Viewport state

        void set_viewport_count(std::uint32_t viewport_count);
        void set_scissor_count(std::uint32_t scissor_count);

        // Rasterization state

        void set_polygon_mode(VkPolygonMode polygon_mode);
        void set_cull_mode(VkCullModeFlags cull_mode);
        void set_front_face(VkFrontFace front_face);

        // Build the create info
        tl::expected<VkPipeline, VkResult> build(VkDevice device);

    private:
        static constexpr auto dynamic_states = std::array{
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        std::vector<VkPipelineShaderStageCreateInfo> shader_stages_;

        VkShaderModuleCreateInfo vs_info_;
        VkShaderModuleCreateInfo fs_info_;
        std::vector<std::uint32_t> vs_code_;
        std::vector<std::uint32_t> fs_code_;

        VkPipelineRenderingCreateInfo rendering_state_;
        std::vector<VkFormat> color_attachments_;

        VkPipelineVertexInputStateCreateInfo vertex_input_state_;
        std::vector<VkVertexInputBindingDescription> vertex_bindings_;
        std::vector<VkVertexInputAttributeDescription> vertex_attributes_;

        VkPipelineInputAssemblyStateCreateInfo input_assembly_state_;
        VkPipelineViewportStateCreateInfo viewport_state_;
        VkPipelineRasterizationStateCreateInfo rasterization_state_;
        VkPipelineMultisampleStateCreateInfo multisample_state_;
        VkPipelineDepthStencilStateCreateInfo depth_stencil_state_;

        VkPipelineColorBlendStateCreateInfo color_blend_state_;
        std::vector<VkPipelineColorBlendAttachmentState> blend_attachments_;

        VkPipelineDynamicStateCreateInfo dynamic_state_;
        VkPipelineLayout layout_;
    };

    template<typename F>
    concept PipelineSetupFn = requires(F setup, PipelineBuilder& builder) {
        setup(builder);
    };

    class PipelineCache
    {
    public:
        static tl::expected<PipelineCache, VkResult> initialize(VkDevice device);
        PipelineCache(const PipelineCache&) = delete;
        PipelineCache& operator=(const PipelineCache&) = delete;
        PipelineCache(PipelineCache&& other) noexcept;
        PipelineCache& operator=(PipelineCache&& other) noexcept;
        ~PipelineCache();

        // Create a new pipeline
        tl::expected<VkPipeline, VkResult> build(std::string name, PipelineSetupFn auto&& setup)
        {
            auto builder = PipelineBuilder{pipeline_layout_};
            setup(builder);
            return build(std::move(name), builder);
        }

        // Retrieve an existing pipeline by identifier
        VkPipeline get(std::string_view name) const;

    private:
        PipelineCache(VkDevice device, VkPipelineLayout pipeline_layout);

        struct PipelineHash {
            using is_transparent = void;

            [[nodiscard]] std::size_t operator()(const std::string& str) const
            {
                return std::hash<std::string>{}(str);
            }
            [[nodiscard]] std::size_t operator()(std::string_view str) const
            {
                return std::hash<std::string_view>{}(str);
            }
        };

        tl::expected<VkPipeline, VkResult> build(std::string name, PipelineBuilder& builder);

        VkDevice vk_device_;
        VkPipelineLayout pipeline_layout_;
        std::unordered_map<std::string, VkPipeline, PipelineHash, std::equal_to<>> pipelines_;
    };
} // namespace orion
