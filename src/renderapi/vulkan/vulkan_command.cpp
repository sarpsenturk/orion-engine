#include "vulkan_command.h"

#include "vulkan_error.h"

#include "orion/assertion.h"

#include <algorithm>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <utility>

namespace orion
{
    namespace
    {
        std::pair<VkPipelineStageFlags, VkPipelineStageFlags> make_transition_stages_flags(ResourceState before, ResourceState after)
        {
            if (before == ResourceState::Present && after == ResourceState::RenderTarget) {
                return std::make_pair(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            } else if (before == ResourceState::RenderTarget && after == ResourceState::Present) {
                return std::make_pair(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
            } else if (before == ResourceState::Unknown && after == ResourceState::RenderTarget) {
                return std::make_pair(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            } else {
                return std::make_pair(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
            }
        }

        std::pair<VkAccessFlags, VkAccessFlags> make_transition_access_flags(ResourceState before, ResourceState after)
        {
            if (before == ResourceState::Present && after == ResourceState::RenderTarget) {
                return std::make_pair(VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
            } else if (before == ResourceState::RenderTarget && after == ResourceState::Present) {
                return std::make_pair(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT);
            } else if (before == ResourceState::Unknown && after == ResourceState::RenderTarget) {
                return std::make_pair(VK_ACCESS_NONE, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
            } else {
                return std::make_pair(VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT);
            }
        }

        std::pair<VkImageLayout, VkImageLayout> make_transition_image_layout(ResourceState before, ResourceState after)
        {
            if (before == ResourceState::Present && after == ResourceState::RenderTarget) {
                return std::make_pair(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            } else if (before == ResourceState::RenderTarget && after == ResourceState::Present) {
                return std::make_pair(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            } else if (before == ResourceState::Unknown && after == ResourceState::RenderTarget) {
                return std::make_pair(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
            } else {
                return std::make_pair(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
            }
        }
    } // namespace

    VulkanCommandList::VulkanCommandList(VkDevice device, VkCommandBuffer command_buffer, std::uint32_t queue_index, VulkanContext* context)
        : device_(device)
        , command_buffer_(command_buffer)
        , queue_index_(queue_index)
        , context_(context)
    {
        ORION_ASSERT(device_ != VK_NULL_HANDLE);
        ORION_ASSERT(command_buffer_ != VK_NULL_HANDLE);
    }

    void VulkanCommandList::begin_api()
    {
        const auto info = VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = {},
            .pInheritanceInfo = nullptr,
        };
        vk_assert(vkBeginCommandBuffer(command_buffer_, &info));
    }

    void VulkanCommandList::end_api()
    {
        vk_assert(vkEndCommandBuffer(command_buffer_));
    }

    void VulkanCommandList::begin_rendering_api(const CmdBeginRendering& cmd)
    {
        // Get Vulkan attachments
        color_attachments_.resize(cmd.render_targets.size());
        std::ranges::transform(cmd.render_targets, color_attachments_.begin(), [this](const RenderAttachment& attachment) {
            VkImageView vk_image_view = context_->lookup(attachment.render_target);
            ORION_ASSERT(vk_image_view != VK_NULL_HANDLE);
            return VkRenderingAttachmentInfoKHR{
                .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
                .pNext = nullptr,
                .imageView = vk_image_view,
                .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                .resolveMode = VK_RESOLVE_MODE_NONE,
                .resolveImageView = VK_NULL_HANDLE,
                .resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = VkClearColorValue{
                    attachment.clear_color[0],
                    attachment.clear_color[1],
                    attachment.clear_color[2],
                    attachment.clear_color[3],
                },
            };
        });

        const auto info = VkRenderingInfoKHR{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .pNext = nullptr,
            .flags = {},
            .renderArea = {
                .offset = {.x = cmd.render_area.x, .y = cmd.render_area.y},
                .extent = {.width = static_cast<uint32_t>(cmd.render_area.width), .height = static_cast<uint32_t>(cmd.render_area.height)},
            },
            .layerCount = 1,
            .viewMask = 0,
            .colorAttachmentCount = static_cast<uint32_t>(color_attachments_.size()),
            .pColorAttachments = color_attachments_.data(),
            .pDepthAttachment = nullptr,
            .pStencilAttachment = nullptr,
        };
        vkCmdBeginRenderingKHR(command_buffer_, &info);
    }

    void VulkanCommandList::end_rendering_api()
    {
        // End rendering
        vkCmdEndRenderingKHR(command_buffer_);

        // Clear data
        color_attachments_.clear();
    }

    void VulkanCommandList::transition_barrier_api(const CmdTransitionBarrier& cmd)
    {
        // Find image with handle
        VkImage vk_image = context_->lookup(cmd.image);
        ORION_ASSERT(vk_image != VK_NULL_HANDLE);

        // Get src & dst pairs of VkPipelineStageFlags, VkAccessFlags & VkImageLayout
        // for cmd.before & cmd.after
        const auto [src_stages, dst_stages] = make_transition_stages_flags(cmd.before, cmd.after);
        const auto [src_access, dst_access] = make_transition_access_flags(cmd.before, cmd.after);
        const auto [old_layout, new_layout] = make_transition_image_layout(cmd.before, cmd.after);

        // Set up image memory barrier for transition
        const auto image_barrier = VkImageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = src_access,
            .dstAccessMask = dst_access,
            .oldLayout = old_layout,
            .newLayout = new_layout,
            .srcQueueFamilyIndex = queue_index_,
            .dstQueueFamilyIndex = queue_index_,
            .image = vk_image,
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, // TODO: This should be customizable
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        // Add the pipeline barrier
        vkCmdPipelineBarrier(
            command_buffer_,
            src_stages, dst_stages,
            {},
            0, nullptr,       // Memory barriers
            0, nullptr,       // Buffer memory barriers
            1, &image_barrier // Image barriers
        );
    }

    void VulkanCommandList::set_pipeline_api(const CmdSetPipeline& cmd)
    {
        // Find pipeline with handle
        VkPipeline vk_pipeline = context_->lookup(cmd.pipeline);
        ORION_ASSERT(vk_pipeline != VK_NULL_HANDLE);

        // Bind the pipeline
        vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);
    }

    void VulkanCommandList::set_viewports_api(const CmdSetViewports& cmd)
    {
        // Convert Viewport to VkViewport
        viewports_.resize(cmd.viewports.size());
        std::ranges::transform(cmd.viewports, viewports_.begin(), [](const Viewport& viewport) {
            return VkViewport{
                .x = viewport.x,
                .y = viewport.y,
                .width = viewport.width,
                .height = viewport.height,
                .minDepth = viewport.min_depth,
                .maxDepth = viewport.max_depth,
            };
        });

        // Set the viewports
        vkCmdSetViewport(command_buffer_, cmd.start_viewport, static_cast<uint32_t>(viewports_.size()), viewports_.data());

        // Cleanup data
        viewports_.clear();
    }

    void VulkanCommandList::set_scissors_api(const CmdSetScissors& cmd)
    {
        // Convert Rect2D to VkRect2D
        scissors_.resize(cmd.scissors.size());
        std::ranges::transform(cmd.scissors, scissors_.begin(), [](const Rect2D& rect) {
            return VkRect2D{
                .offset = {.x = rect.x, .y = rect.y},
                .extent = {.width = static_cast<uint32_t>(rect.width), .height = static_cast<uint32_t>(rect.height)},
            };
        });

        // Set the scissors
        vkCmdSetScissor(command_buffer_, cmd.start_scissor, static_cast<uint32_t>(scissors_.size()), scissors_.data());

        // Cleanup data
        scissors_.clear();
    }

    void VulkanCommandList::set_vertex_buffers_api(const CmdSetVertexBuffers& cmd)
    {
        // Set up vertex buffers and offsets
        vertex_buffers_.reserve(cmd.vertex_buffers.size());
        vertex_buffers_.reserve(cmd.vertex_buffers.size());
        std::ranges::for_each(cmd.vertex_buffers, [this](const BufferView& buffer_view) {
            VulkanBuffer buffer = context_->lookup(buffer_view.buffer);
            ORION_ASSERT(buffer.buffer != VK_NULL_HANDLE);
            vertex_buffers_.push_back(buffer.buffer);
            vertex_buffer_offsets_.push_back(0);
        });

        // Bind vertex buffers
        vkCmdBindVertexBuffers(
            command_buffer_,
            cmd.start_buffer,
            static_cast<uint32_t>(vertex_buffers_.size()),
            vertex_buffers_.data(),
            vertex_buffer_offsets_.data());

        // Clear data
        vertex_buffers_.clear();
        vertex_buffer_offsets_.clear();
    }

    void VulkanCommandList::draw_instanced_api(const CmdDrawInstanced& cmd)
    {
        // Make draw call
        vkCmdDraw(command_buffer_, cmd.vertex_count, cmd.instance_count, cmd.start_vertex, cmd.start_instance);
    }

    VulkanCommandAllocator::VulkanCommandAllocator(VkDevice device, VkCommandPool command_pool)
        : device_(device)
        , command_pool_(command_pool)
    {
        ORION_ASSERT(device_ != VK_NULL_HANDLE);
        ORION_ASSERT(command_pool_ != VK_NULL_HANDLE);
    }

    VulkanCommandAllocator::~VulkanCommandAllocator()
    {
        ORION_ASSERT(command_pool_ != VK_NULL_HANDLE);
        vkDestroyCommandPool(device_, command_pool_, nullptr);
    }

    void VulkanCommandAllocator::reset_api()
    {
        vk_assert(vkResetCommandPool(device_, command_pool_, {}));
    }
} // namespace orion
