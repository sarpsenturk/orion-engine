#include "vulkan_command.h"

#include "vulkan_conversion.h"
#include "vulkan_device.h"

namespace orion::vulkan
{
    VulkanCommandList::VulkanCommandList(VulkanDevice* device, UniqueVkCommandBuffer command_buffer)
        : device_(device)
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
        VkBuffer vk_buffer = device_->buffers().handle_at(cmd_bind_index_buffer.index_buffer);
        vkCmdBindIndexBuffer(
            command_buffer_.get(),
            vk_buffer,
            VkDeviceSize{cmd_bind_index_buffer.offset},
            to_vulkan_type(cmd_bind_index_buffer.index_type));
    }

    void VulkanCommandList::bind_vertex_buffer_api(const CmdBindVertexBuffer& cmd_bind_vertex_buffer)
    {
        VkBuffer vk_buffer = device_->buffers().handle_at(cmd_bind_vertex_buffer.vertex_buffer);
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
        VkPipeline vk_pipeline = device_->pipelines().handle_at(cmd_bind_pipeline.pipeline);
        vkCmdBindPipeline(
            command_buffer_.get(),
            to_vulkan_type(cmd_bind_pipeline.bind_point),
            vk_pipeline);
    }

    VulkanCommandAllocator::VulkanCommandAllocator(VulkanDevice* device, UniqueVkCommandPool command_pool)
        : device_(device)
        , command_pool_(std::move(command_pool))
    {
    }

    void VulkanCommandAllocator::reset_api()
    {
        vk_result_check(vkResetCommandPool(device_->device(), command_pool_.get(), 0));
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
            vk_result_check(vkAllocateCommandBuffers(device_->device(), &info, &command_buffer));
        }
        return std::make_unique<VulkanCommandList>(device_, unique(command_buffer, device_->device(), command_pool_.get()));
    }
} // namespace orion::vulkan
