#pragma once

#include <volk.h>

#include <vk_mem_alloc.h>

#include <concepts>
#include <cstdint>
#include <functional>
#include <map>
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
        RenderPassContext(class RenderGraph& graph, VkCommandBuffer command_buffer);

        class RenderGraph& graph_;
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
        DepthAttachment,
    };

    class RenderPassBuilder
    {
    public:
        TextureHandle write_texture(TextureHandle handle, TextureUsage usage);

    private:
        friend class RenderGraph;
        explicit RenderPassBuilder(RenderPass& pass);

        RenderPass& pass_;
    };

    template<typename F>
    concept RenderPassSetupFn = requires(F setup, RenderPassBuilder& builder) {
        { setup(builder) } -> std::convertible_to<RenderPassExecuteFn>;
    };

    class RenderGraph
    {
    public:
        enum class TextureLifetime {
            Transient,
            Persistent,
        };

        struct TextureImportDesc {
            VkImage image;
            VkImageView view;
            VkImageLayout current_layout;
            VkImageLayout final_layout;
            VkFormat format;
        };

        struct TextureDesc {
            VkImageType image_type;
            VkFormat format;
            VkExtent3D extent;
            VkImageUsageFlags usage;

            [[nodiscard]] constexpr friend bool operator<(const TextureDesc& lhs, const TextureDesc& rhs) noexcept
            {
                if (lhs.image_type != rhs.image_type) {
                    return lhs.image_type < rhs.image_type;
                }
                if (lhs.format != rhs.format) {
                    return lhs.format < rhs.format;
                }
                if (lhs.extent.width != rhs.extent.width) {
                    return lhs.extent.width < rhs.extent.width;
                }
                if (lhs.extent.height != rhs.extent.height) {
                    return lhs.extent.height < rhs.extent.height;
                }
                if (lhs.extent.depth != rhs.extent.depth) {
                    return lhs.extent.depth < rhs.extent.depth;
                }
                return lhs.usage < rhs.usage;
            }
        };

        struct Texture {
            TextureLifetime lifetime = TextureLifetime::Transient;

            VkImage image = VK_NULL_HANDLE;
            VkImageView image_view = VK_NULL_HANDLE;

            VkImageLayout current_layout = VK_IMAGE_LAYOUT_UNDEFINED;
            VkPipelineStageFlags2 last_stage = VK_PIPELINE_STAGE_2_NONE;
            VkAccessFlags2 last_access = VK_ACCESS_2_NONE;

            VkImageLayout final_layout = VK_IMAGE_LAYOUT_UNDEFINED;

            TextureDesc desc;
        };

        struct TextureAllocation {
            VkImage image;
            VkImageView view;
            VmaAllocation allocation;
            std::uint32_t frames_since_use = 0;
        };

        RenderGraph() = default;
        RenderGraph(VkDevice device, VmaAllocator allocator);
        RenderGraph(const RenderGraph&) = delete;
        RenderGraph& operator=(const RenderGraph&) = delete;
        RenderGraph(RenderGraph&&) noexcept = default;
        RenderGraph& operator=(RenderGraph&&) noexcept = default;
        ~RenderGraph();

        TextureHandle import_texture(const TextureImportDesc& desc);
        TextureHandle create_transient_texture(const TextureDesc& desc);

        const Texture& get_texture(TextureHandle handle) const;

        void add_pass(std::string name, const RenderPassSetupFn auto& setup)
        {
            passes_.emplace_back(std::move(name));
            auto& pass = passes_.back();

            auto builder = RenderPassBuilder{pass};
            pass.execute = setup(builder);
        }
        void compile();
        void execute(VkCommandBuffer command_buffer);
        void reset();

    private:
        void compile_sort_passes();
        void compile_increment_transient_resource_last_use();
        void compile_allocate_transient_resources();
        void compile_emit_pass_barriers();
        void compile_emit_final_layout_transitions();

        VkDevice vk_device_ = VK_NULL_HANDLE;
        VmaAllocator vma_allocator_ = VK_NULL_HANDLE;
        std::vector<Texture> textures_;
        std::vector<RenderPass> passes_;
        std::vector<std::size_t> sorted_passes_;
        std::vector<VkImageMemoryBarrier2> final_layout_transitions_;
        std::map<TextureDesc, TextureAllocation> transient_textures_;
    };
} // namespace orion
