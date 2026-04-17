#include "orion/renderer/render_graph.hpp"

#include "orion/debug.hpp"
#include "orion/log.hpp"

#include "vulkan_impl.hpp"
#include <vulkan/vk_enum_string_helper.h>

#include <fmt/format.h>

#include <stdexcept>

namespace orion
{
    static TextureAccess to_write_texture_access(TextureHandle handle, TextureUsage usage)
    {
        switch (usage) {
            case TextureUsage::ColorAttachment:
                return {
                    .handle = handle,
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                    .stage = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
                    .access = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
                };
            case TextureUsage::DepthAttachment:
                return {
                    .handle = handle,
                    .layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
                    .stage = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
                    .access = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                };
        }
        unreachable();
    }

    static bool is_write_access(VkAccessFlags2 access_flags)
    {
        static constexpr auto write_mask = VK_ACCESS_2_SHADER_WRITE_BIT |
                                           VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT |
                                           VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                           VK_ACCESS_2_TRANSFER_WRITE_BIT |
                                           VK_ACCESS_2_HOST_WRITE_BIT |
                                           VK_ACCESS_2_MEMORY_WRITE_BIT |
                                           VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
        return (access_flags & write_mask) != 0;
    }

    static bool requires_image_barrier(const TextureAccess& src, const TextureAccess& dst)
    {
        const auto is_layout_change = src.layout != dst.layout;
        const auto is_write = is_write_access(src.access) || is_write_access(dst.access);
        return is_layout_change || is_write;
    }

    static VkImageAspectFlags to_image_aspect_flags(VkFormat format)
    {
        ORION_ASSERT(format != VK_FORMAT_UNDEFINED);
        switch (format) {
            case VK_FORMAT_D32_SFLOAT:
                return VK_IMAGE_ASPECT_DEPTH_BIT;
            default:
                return VK_IMAGE_ASPECT_COLOR_BIT;
        }
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

    RenderPassBuilder::RenderPassBuilder(RenderPass& pass)
        : pass_(pass)
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
        // Free all transient resources that were not used in the last frame
        for (auto it = transient_textures_.begin(); it != transient_textures_.end();) {
            if (it->second.frames_since_use >= 1) {
                vkDestroyImageView(vk_device_, it->second.view, nullptr);
                ORION_RENDERER_LOG_INFO("Destroyed VkImageView {}", fmt::ptr(it->second.view));
                vmaDestroyImage(vma_allocator_, it->second.image, it->second.allocation);
                ORION_RENDERER_LOG_INFO("Destroyed VkImage {} with VmaAllocation {} (unused in previous frame)", fmt::ptr(it->second.image), fmt::ptr(it->second.allocation));
                it = transient_textures_.erase(it);
            } else {
                ++it;
            }
        }

        final_layout_transitions_.clear();
        sorted_passes_.clear();
        passes_.clear();
        textures_.clear();
    }

    RenderGraph::RenderGraph(VkDevice device, VmaAllocator allocator)
        : vk_device_(device)
        , vma_allocator_(allocator)
    {
    }

    RenderGraph::~RenderGraph()
    {
        for (const auto& [_, texture] : transient_textures_) {
            vkDestroyImageView(vk_device_, texture.view, nullptr);
            ORION_RENDERER_LOG_INFO("Destroyed VkImageView {}", fmt::ptr(texture.view));
            vmaDestroyImage(vma_allocator_, texture.image, texture.allocation);
            ORION_RENDERER_LOG_INFO("Destroyed VkImage {} with VmaAllocation {}", fmt::ptr(texture.image), fmt::ptr(texture.allocation));
        }
    }

    TextureHandle RenderGraph::import_texture(const TextureImportDesc& desc)
    {
        const auto index = static_cast<std::uint16_t>(textures_.size());
        textures_.push_back(Texture{
            .lifetime = TextureLifetime::Persistent,
            .image = desc.image,
            .image_view = desc.view,
            .current_layout = desc.current_layout,
            .last_stage = VK_PIPELINE_STAGE_2_NONE,
            .last_access = VK_ACCESS_2_NONE,
            .final_layout = desc.final_layout,
            .desc = {
                .format = desc.format,
            },
        });
        return {index, 0};
    }

    TextureHandle RenderGraph::create_transient_texture(const TextureDesc& desc)
    {
        const auto index = static_cast<std::uint16_t>(textures_.size());
        textures_.push_back(Texture{
            .lifetime = TextureLifetime::Transient,
            .desc = desc,
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
        compile_increment_transient_resource_last_use();
        compile_allocate_transient_resources();
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

    void RenderGraph::compile_increment_transient_resource_last_use()
    {
        for (auto& [_, texture] : transient_textures_) {
            ++texture.frames_since_use;
        }
    }

    void RenderGraph::compile_allocate_transient_resources()
    {
        for (auto& texture : textures_) {
            // If imported texture, skip it
            if (texture.lifetime == TextureLifetime::Persistent) {
                continue;
            }

            // Check if texture was cached
            if (auto it = transient_textures_.find(texture.desc); it != transient_textures_.end()) {
                texture.image = it->second.image;
                texture.image_view = it->second.view;
                // Mark resource as used in this frame
                it->second.frames_since_use = 0;
            } else {
                // Allocate image
                const auto image_info = VkImageCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .imageType = texture.desc.image_type,
                    .format = texture.desc.format,
                    .extent = texture.desc.extent,
                    .mipLevels = 1,
                    .arrayLayers = 1,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .tiling = VK_IMAGE_TILING_OPTIMAL,
                    .usage = texture.desc.usage,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .queueFamilyIndexCount = 0,
                    .pQueueFamilyIndices = nullptr,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                };
                const auto allocation_info = VmaAllocationCreateInfo{
                    .usage = VMA_MEMORY_USAGE_AUTO,
                };
                VkImage image = VK_NULL_HANDLE;
                VmaAllocation allocation = VK_NULL_HANDLE;
                if (VkResult err = vmaCreateImage(vma_allocator_, &image_info, &allocation_info, &image, &allocation, nullptr)) {
                    throw std::runtime_error(fmt::format("vmaCreateImage() failed: {}", string_VkResult(err)));
                } else {
                    ORION_RENDERER_LOG_INFO("Created VkImage {} with VmaAllocation {}", fmt::ptr(image), fmt::ptr(allocation));
                }

                // Create image view
                const auto image_view_info = VkImageViewCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = {},
                    .image = image,
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = texture.desc.format,
                    .components = {}, // VK_COMPONENT_SWIZZLE_IDENTITY
                    .subresourceRange = {
                        .aspectMask = to_image_aspect_flags(texture.desc.format),
                        .baseMipLevel = 0,
                        .levelCount = VK_REMAINING_MIP_LEVELS,
                        .baseArrayLayer = 0,
                        .layerCount = VK_REMAINING_ARRAY_LAYERS,
                    },
                };
                VkImageView view = VK_NULL_HANDLE;
                if (VkResult err = vkCreateImageView(vk_device_, &image_view_info, nullptr, &view)) {
                    vmaDestroyImage(vma_allocator_, image, allocation);
                    throw std::runtime_error(fmt::format("vkCreateImageView() failed: {}", string_VkResult(err)));
                } else {
                    ORION_RENDERER_LOG_INFO("Created VkImageView {}", fmt::ptr(view));
                }

                // Insert into cache
                transient_textures_.emplace(std::make_pair(texture.desc, TextureAllocation{image, view, allocation}));

                // Mark non-virtual resource
                texture.image = image;
                texture.image_view = view;
            }
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
                            .aspectMask = to_image_aspect_flags(entry.desc.format),
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
            if (texture.lifetime == TextureLifetime::Persistent &&
                texture.current_layout != texture.final_layout) {
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
