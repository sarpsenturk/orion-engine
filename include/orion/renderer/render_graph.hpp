#pragma once

#include <volk.h>

#include <concepts>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace orion
{
    struct TextureHandle {
        std::uint16_t index;
        std::uint16_t version = 0;
    };

    class RenderPassContext
    {
    public:
        VkCommandBuffer cmd() const { return command_buffer_; }
        VkImageView get_image_view(TextureHandle handle) const;

    private:
        friend class RenderGraph;
        RenderPassContext(RenderGraph& graph, VkCommandBuffer command_buffer);

        RenderGraph& graph_;
        VkCommandBuffer command_buffer_;
    };
    using RenderPassExecuteFn = std::function<void(RenderPassContext&)>;

    struct TextureAccess {
        TextureHandle handle;
        VkImageLayout layout;
        VkPipelineStageFlags2 stage;
        VkAccessFlags2 access;
    };

    struct RenderPass {
        std::string name;
        RenderPassExecuteFn execute;
        std::vector<TextureAccess> texture_accesses;
        std::vector<VkImageMemoryBarrier2> image_barriers;
    };

    enum class TextureUsage {
        ColorAttachment,
    };

    class RenderPassBuilder
    {
    public:
        TextureHandle write_texture(TextureHandle handle, TextureUsage usage);

    private:
        friend class RenderGraph;
        RenderPassBuilder(RenderGraph& graph, RenderPass& pass);

        RenderGraph& graph_;
        RenderPass& pass_;
    };

    template<typename F>
    concept RenderPassSetupFn = requires(F setup, RenderPassBuilder& builder) {
        { setup(builder) } -> std::convertible_to<RenderPassExecuteFn>;
    };

    class RenderGraph
    {
    public:
        struct Texture {
            bool imported = false;

            VkImage image = VK_NULL_HANDLE;
            VkImageView image_view = VK_NULL_HANDLE;

            VkImageLayout current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkPipelineStageFlags2 last_stage = VK_PIPELINE_STAGE_2_NONE;
            VkAccessFlags2 last_access = VK_ACCESS_2_NONE;

            VkImageLayout final_layout = VK_IMAGE_LAYOUT_UNDEFINED;
        };

        TextureHandle import_texture(VkImage image, VkImageView view, VkImageLayout current_layout, VkImageLayout final_layout);
        const Texture& get_texture(TextureHandle handle) const;

        void add_pass(std::string name, const RenderPassSetupFn auto& setup)
        {
            passes_.emplace_back(std::move(name));
            auto& pass = passes_.back();

            auto builder = RenderPassBuilder{*this, pass};
            pass.execute = setup(builder);
        }
        void compile();
        void execute(VkCommandBuffer command_buffer);
        void reset();

    private:
        void compile_sort_passes();
        void compile_emit_pass_barriers();
        void compile_emit_final_layout_transitions();

        std::vector<Texture> textures_;
        std::vector<RenderPass> passes_;
        std::vector<std::size_t> sorted_passes_;
        std::vector<VkImageMemoryBarrier2> final_layout_transitions_;
    };
} // namespace orion
