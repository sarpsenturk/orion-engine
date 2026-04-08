#include "orion/renderer/render_graph.hpp"

#include "orion/debug.hpp"

namespace orion
{
    static TextureAccess to_write_texture_access(TextureHandle handle, TextureUsage usage)
    {
        switch (usage) {
            case TextureUsage::ColorAttachment:
                return TextureAccess{
                    .handle = handle,
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .access = VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT,
                };
        }
        unreachable();
    }

    static bool is_write_access(VkAccessFlags2 access_flags)
    {
        static constexpr auto write_mask = VK_ACCESS_2_SHADER_WRITE_BIT &
                                           VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT &
                                           VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT &
                                           VK_ACCESS_2_TRANSFER_WRITE_BIT &
                                           VK_ACCESS_2_HOST_WRITE_BIT &
                                           VK_ACCESS_2_MEMORY_WRITE_BIT &
                                           VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        return !!(access_flags & write_mask);
    }

    static bool requires_image_barrier(const TextureAccess& src, const TextureAccess& dst)
    {
        const auto is_layout_change = src.layout != dst.layout;
        const auto is_write = is_write_access(src.access) || is_write_access(dst.access);
        return is_layout_change || is_write;
    }

    RenderPassContext::RenderPassContext(RenderGraph& graph, VkCommandBuffer command_buffer)
        : graph_(graph)
        , command_buffer_(command_buffer)
    {
    }

    VkImageView RenderPassContext::get_image_view(TextureHandle handle) const
    {
        return graph_.get_texture(handle).image_view;
    }

    RenderPassBuilder::RenderPassBuilder(RenderGraph& graph, RenderPass& pass)
        : graph_(graph)
        , pass_(pass)
    {
    }

    TextureHandle RenderPassBuilder::write_texture(TextureHandle handle, TextureUsage usage)
    {
        pass_.texture_accesses.push_back(to_write_texture_access(handle, usage));
        ++handle.version;
        return handle;
    }

    void RenderGraph::reset()
    {
        final_layout_transitions_.clear();
        sorted_passes_.clear();
        passes_.clear();
        textures_.clear();
    }

    TextureHandle RenderGraph::import_texture(VkImage image, VkImageView view, VkImageLayout current_layout, VkImageLayout final_layout)
    {
        const auto index = static_cast<std::uint16_t>(textures_.size());
        textures_.push_back(Texture{
            .imported = true,
            .image = image,
            .image_view = view,
            .current_layout = current_layout,
            .last_stage = VK_PIPELINE_STAGE_2_NONE,
            .last_access = VK_ACCESS_2_NONE,
            .final_layout = final_layout,
        });
        return {index, 0};
    }

    const RenderGraph::Texture& RenderGraph::get_texture(TextureHandle handle) const
    {
        ORION_ASSERT(handle.index < textures_.size());
        return textures_[handle.index];
    }

    void RenderGraph::compile()
    {
        compile_sort_passes();
        compile_emit_pass_barriers();
        compile_emit_final_layout_transitions();
    }

    void RenderGraph::execute(VkCommandBuffer command_buffer)
    {
        auto context = RenderPassContext{*this, command_buffer};
        for (auto pass_idx : sorted_passes_) {
            auto& pass = passes_[pass_idx];
            if (!pass.image_barriers.empty()) {
                const auto dependency_info = VkDependencyInfo{
                    .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                    .pNext = nullptr,
                    .imageMemoryBarrierCount = static_cast<std::uint32_t>(pass.image_barriers.size()),
                    .pImageMemoryBarriers = pass.image_barriers.data(),
                };
                vkCmdPipelineBarrier2(command_buffer, &dependency_info);
            }
            pass.execute(context);
        }

        // Final layout transitions for imported textures
        if (!final_layout_transitions_.empty()) {
            const auto dependency_info = VkDependencyInfo{
                .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                .pNext = nullptr,
                .imageMemoryBarrierCount = static_cast<std::uint32_t>(final_layout_transitions_.size()),
                .pImageMemoryBarriers = final_layout_transitions_.data(),
            };
            vkCmdPipelineBarrier2(command_buffer, &dependency_info);
        }
    }

    void RenderGraph::compile_sort_passes()
    {
        sorted_passes_.resize(passes_.size());
        for (std::size_t i = 0; i < passes_.size(); ++i) {
            sorted_passes_[i] = i;
        }
    }

    void RenderGraph::compile_emit_pass_barriers()
    {
        for (auto pass_idx : sorted_passes_) {
            auto& pass = passes_[pass_idx];
            for (const auto& access : pass.texture_accesses) {
                auto& entry = textures_[access.handle.index];
                const auto src = TextureAccess{.layout = entry.current_layout, .stage = entry.last_stage, .access = entry.last_access};
                if (requires_image_barrier(src, access)) {
                    pass.image_barriers.push_back(VkImageMemoryBarrier2{
                        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                        .pNext = nullptr,
                        .srcStageMask = src.stage,
                        .srcAccessMask = src.access,
                        .dstStageMask = access.stage,
                        .dstAccessMask = access.access,
                        .oldLayout = src.layout,
                        .newLayout = access.layout,
                        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                        .image = entry.image,
                        .subresourceRange = {
                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                            .baseMipLevel = 0,
                            .levelCount = 1,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                        },
                    });
                }
                entry.current_layout = access.layout;
                entry.last_stage = access.stage;
                entry.last_access = access.access;
            }
        }
    }

    void RenderGraph::compile_emit_final_layout_transitions()
    {
        for (const auto& texture : textures_) {
            if (texture.imported && texture.current_layout != texture.final_layout) {
                final_layout_transitions_.push_back(VkImageMemoryBarrier2{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
                    .pNext = nullptr,
                    .srcStageMask = texture.last_stage,
                    .srcAccessMask = texture.last_access,
                    .dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT,
                    .dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT | VK_ACCESS_2_MEMORY_WRITE_BIT,
                    .oldLayout = texture.current_layout,
                    .newLayout = texture.final_layout,
                    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                    .image = texture.image,
                    .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = 1,
                        .baseArrayLayer = 0,
                        .layerCount = 1,
                    },
                });
            }
        }
    }
} // namespace orion
