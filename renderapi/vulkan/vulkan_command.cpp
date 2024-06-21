#include "vulkan_command.h"

#include "vulkan_conversion.h"
#include "vulkan_device.h"
#include "vulkan_renderpass.h"

#include <array>
#include <ranges>

namespace orion::vulkan
{
    namespace
    {
        std::pair<VkAccessFlags, VkAccessFlags> access_flags_for_transition(VkImageLayout old_layout, VkImageLayout new_layout)
        {
            if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                return std::make_pair(0, VK_ACCESS_TRANSFER_WRITE_BIT);
            }
            if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                return std::make_pair(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
            }
            if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                return std::make_pair(0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
            }
            if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                return std::make_pair(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
            }
            if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
                return std::make_pair(VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0);
            }
            throw std::invalid_argument("invalid vulkan transition");
        }

        std::pair<VkPipelineStageFlags, VkPipelineStageFlags> stage_flags_for_transition(VkImageLayout old_layout, VkImageLayout new_layout)
        {
            if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                return std::make_pair(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
            }
            if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                return std::make_pair(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            }
            if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
                return std::make_pair(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            }
            if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                return std::make_pair(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
            }
            if (old_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR) {
                return std::make_pair(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
            }
            throw std::invalid_argument("invalid vulkan transition");
        }
    } // namespace

    VulkanCommandList::VulkanCommandList(VulkanResourceManager* resource_manager, UniqueVkCommandBuffer command_buffer)
        : resource_manager_(resource_manager)
        , command_buffer_(std::move(command_buffer))
    {
    }

    void VulkanCommandList::begin_api()
    {
        const auto info = VkCommandBufferBeginInfo{
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .pNext = nullptr,
            .flags = 0,
            .pInheritanceInfo = nullptr,
        };
        vk_result_check(vkBeginCommandBuffer(command_buffer_.get(), &info));
    }

    void VulkanCommandList::end_api()
    {
        vk_result_check(vkEndCommandBuffer(command_buffer_.get()));
    }

    void VulkanCommandList::reset_api()
    {
        vk_result_check(vkResetCommandBuffer(command_buffer_.get(), 0));
    }

    void VulkanCommandList::draw_api(const CmdDraw& cmd_draw)
    {
        vkCmdDraw(
            command_buffer_.get(),
            cmd_draw.vertex_count,
            cmd_draw.instance_count,
            cmd_draw.first_vertex,
            cmd_draw.first_instance);
    }

    void VulkanCommandList::draw_indexed_api(const CmdDrawIndexed& cmd_draw_indexed)
    {
        vkCmdDrawIndexed(
            command_buffer_.get(),
            cmd_draw_indexed.index_count,
            cmd_draw_indexed.instance_count,
            cmd_draw_indexed.first_index,
            cmd_draw_indexed.vertex_offset,
            cmd_draw_indexed.first_instance);
    }

    void VulkanCommandList::bind_index_buffer_api(const CmdBindIndexBuffer& cmd_bind_index_buffer)
    {
        VkBuffer vk_buffer = resource_manager_->find(cmd_bind_index_buffer.index_buffer).buffer;
        vkCmdBindIndexBuffer(
            command_buffer_.get(),
            vk_buffer,
            VkDeviceSize{cmd_bind_index_buffer.offset},
            to_vulkan_type(cmd_bind_index_buffer.index_type));
    }

    void VulkanCommandList::bind_vertex_buffer_api(const CmdBindVertexBuffer& cmd_bind_vertex_buffer)
    {
        VkBuffer vk_buffer = resource_manager_->find(cmd_bind_vertex_buffer.vertex_buffer).buffer;
        const auto offset = VkDeviceSize{cmd_bind_vertex_buffer.offset};
        vkCmdBindVertexBuffers(
            command_buffer_.get(),
            0,
            1,
            &vk_buffer,
            &offset);
    }

    void VulkanCommandList::bind_pipeline_api(const CmdBindPipeline& cmd_bind_pipeline)
    {
        VkPipeline vk_pipeline = resource_manager_->find(cmd_bind_pipeline.pipeline);
        vkCmdBindPipeline(
            command_buffer_.get(),
            to_vulkan_type(cmd_bind_pipeline.bind_point),
            vk_pipeline);
    }

    void VulkanCommandList::bind_descriptor_api(const CmdBindDescriptor& cmd_bind_descriptor)
    {
        VkPipelineLayout layout = resource_manager_->find(cmd_bind_descriptor.pipeline_layout);
        VkDescriptorSet descriptor_set = resource_manager_->find(cmd_bind_descriptor.descriptor);
        vkCmdBindDescriptorSets(
            command_buffer_.get(),                          // command buffer
            to_vulkan_type(cmd_bind_descriptor.bind_point), // pipeline bind point
            layout,                                         // pipeline layout
            cmd_bind_descriptor.index,                      // first set
            1,                                              // set count
            &descriptor_set,                                // descriptor set
            0,                                              // offset count
            nullptr                                         // offsets
        );
    }

    void VulkanCommandList::begin_render_pass_api(const CmdBeginRenderPass& cmd_begin_render_pass)
    {
        ORION_ASSERT(cmd_begin_render_pass.render_pass != nullptr);
        const auto* vulkan_render_pass = static_cast<const VulkanRenderPass*>(cmd_begin_render_pass.render_pass);
        const auto& color_attachments = vulkan_render_pass->color_attachments();

        const auto rendering_info = VkRenderingInfoKHR{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .pNext = nullptr,
            .flags = 0,
            .renderArea = to_vulkan_rect(cmd_begin_render_pass.render_area),
            .layerCount = 1,
            .viewMask = 0,
            .colorAttachmentCount = static_cast<uint32_t>(color_attachments.size()),
            .pColorAttachments = color_attachments.data(),
            .pDepthAttachment = nullptr,
            .pStencilAttachment = nullptr,
        };
        vkCmdBeginRenderingKHR(command_buffer_.get(), &rendering_info);
    }

    void VulkanCommandList::end_render_pass_api()
    {
        vkCmdEndRenderingKHR(command_buffer_.get());
    }

    void VulkanCommandList::set_viewports_api(const CmdSetViewports& cmd_set_viewports)
    {
        std::vector<VkViewport> viewports(cmd_set_viewports.viewports.size());
        std::ranges::transform(cmd_set_viewports.viewports, viewports.begin(), to_vulkan_viewport);
        vkCmdSetViewport(
            command_buffer_.get(),
            cmd_set_viewports.first_viewport,
            static_cast<uint32_t>(viewports.size()),
            viewports.data());
    }

    void VulkanCommandList::set_scissors_api(const CmdSetScissors& cmd_set_scissors)
    {
        std::vector<VkRect2D> scissors(cmd_set_scissors.scissors.size());
        std::ranges::transform(cmd_set_scissors.scissors, scissors.begin(), to_vulkan_scissor);
        vkCmdSetScissor(
            command_buffer_.get(),
            cmd_set_scissors.first_scissor,
            static_cast<uint32_t>(scissors.size()),
            scissors.data());
    }

    void VulkanCommandList::copy_buffer_api(const CmdCopyBuffer& cmd_copy_buffer)
    {
        VkBuffer src = resource_manager_->find(cmd_copy_buffer.src).buffer;
        ORION_EXPECTS(src != VK_NULL_HANDLE);
        VkBuffer dst = resource_manager_->find(cmd_copy_buffer.dst).buffer;

        const auto region = VkBufferCopy{
            .srcOffset = cmd_copy_buffer.src_offset,
            .dstOffset = cmd_copy_buffer.dst_offset,
            .size = cmd_copy_buffer.size,
        };
        vkCmdCopyBuffer(vk_command_buffer(), src, dst, 1u, &region);
    }

    void VulkanCommandList::copy_buffer_to_image_api(const CmdCopyBufferToImage& cmd_copy_buffer_to_image)
    {
        VkBuffer src_buffer = resource_manager_->find(cmd_copy_buffer_to_image.src_buffer).buffer;
        ORION_EXPECTS(src_buffer != VK_NULL_HANDLE);
        VkImage dst_image = resource_manager_->find(cmd_copy_buffer_to_image.dst_image).image;
        ORION_EXPECTS(dst_image != VK_NULL_HANDLE);
        VkImageLayout dst_layout = to_vulkan_type(cmd_copy_buffer_to_image.dst_layout);
        const auto region = VkBufferImageCopy{
            .bufferOffset = cmd_copy_buffer_to_image.buffer_offset,
            .bufferRowLength = 0,
            .bufferImageHeight = 0,
            .imageSubresource = {
                // TODO: Make customizable
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
            .imageExtent = to_vulkan_extent(cmd_copy_buffer_to_image.dst_size),
        };
        vkCmdCopyBufferToImage(
            command_buffer_.get(),
            src_buffer,
            dst_image,
            dst_layout,
            1u,
            &region);
    }

    void VulkanCommandList::push_constants_api(const CmdPushConstants& cmd_push_constants)
    {
        VkPipelineLayout pipeline_layout = resource_manager_->find(cmd_push_constants.pipeline_layout);
        ORION_EXPECTS(pipeline_layout != VK_NULL_HANDLE);
        vkCmdPushConstants(
            command_buffer_.get(),
            pipeline_layout,
            to_vulkan_type(cmd_push_constants.shader_stages),
            cmd_push_constants.offset,
            cmd_push_constants.size,
            cmd_push_constants.values);
    }

    void VulkanCommandList::transition_barrier_api(const CmdTransitionBarrier& cmd_transition_barrier)
    {
        const auto old_layout = to_vulkan_type(cmd_transition_barrier.old_layout);
        const auto new_layout = to_vulkan_type(cmd_transition_barrier.new_layout);
        const auto [src_access, dst_access] = access_flags_for_transition(old_layout, new_layout);
        VkImage image = resource_manager_->find(cmd_transition_barrier.image).image;
        ORION_EXPECTS(image != VK_NULL_HANDLE);
        const auto image_barrier = VkImageMemoryBarrier{
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .pNext = nullptr,
            .srcAccessMask = src_access,
            .dstAccessMask = dst_access,
            .oldLayout = old_layout,
            .newLayout = new_layout,
            .image = image,
            // TODO: Custom subresource
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };
        const auto [src_stages, dst_stages] = stage_flags_for_transition(old_layout, new_layout);
        vkCmdPipelineBarrier(
            command_buffer_.get(),
            src_stages,                  // Src stage mask
            dst_stages,                  // Dst stage mask
            VK_DEPENDENCY_BY_REGION_BIT, // Dependency flags
            0, nullptr,                  // Memory barriers
            0, nullptr,                  // Buffer memory barriers
            1, &image_barrier            // Image barriers
        );
    }

    VulkanCommandAllocator::VulkanCommandAllocator(VkDevice device, VulkanResourceManager* resource_manager, UniqueVkCommandPool command_pool)
        : device_(device)
        , resource_manager_(resource_manager)
        , command_pool_(std::move(command_pool))
    {
    }

    void VulkanCommandAllocator::reset_api()
    {
        vk_result_check(vkResetCommandPool(device_, command_pool_.get(), 0));
    }

    std::unique_ptr<CommandList> VulkanCommandAllocator::create_command_list_api()
    {
        VkCommandBuffer command_buffer = VK_NULL_HANDLE;
        {
            const auto info = VkCommandBufferAllocateInfo{
                .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                .pNext = nullptr,
                .commandPool = command_pool_.get(),
                .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                .commandBufferCount = 1,
            };
            vk_result_check(vkAllocateCommandBuffers(device_, &info, &command_buffer));
        }
        return std::make_unique<VulkanCommandList>(resource_manager_, unique(command_buffer, device_, command_pool_.get()));
    }
} // namespace orion::vulkan
