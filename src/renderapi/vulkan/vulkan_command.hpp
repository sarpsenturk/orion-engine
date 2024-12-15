#pragma once

#include "orion/renderapi/render_command.hpp"

#include "vulkan_context.hpp"

#include <Volk/volk.h>

#include <vector>

namespace orion
{
    class VulkanCommandList final : public CommandList
    {
    public:
        VulkanCommandList(VkDevice device, VkCommandBuffer command_buffer, std::uint32_t queue_index, VulkanContext* context);

        [[nodiscard]] VkCommandBuffer vk_command_buffer() const { return command_buffer_; }

    private:
        void begin_api() override;
        void end_api() override;

        void begin_rendering_api(const CmdBeginRendering& cmd) override;
        void end_rendering_api() override;

        void transition_barrier_api(const CmdTransitionBarrier& cmd) override;

        void set_pipeline_api(const CmdSetPipeline& cmd) override;
        void set_viewports_api(const CmdSetViewports& cmd) override;
        void set_scissors_api(const CmdSetScissors& cmd) override;
        void set_vertex_buffers_api(const CmdSetVertexBuffers& cmd) override;
        void set_index_buffer_api(const CmdSetIndexBuffer& cmd) override;
        void set_bind_group_api(const CmdSetBindGroup& cmd) override;

        void draw_instanced_api(const CmdDrawInstanced& cmd) override;
        void draw_indexed_instanced_api(const CmdDrawIndexedInstanced& cmd) override;

        VkDevice device_;
        VkCommandBuffer command_buffer_;
        std::uint32_t queue_index_;
        VulkanContext* context_;

        // Store vectors for resources in the object
        // to avoid reallocation.
        // TODO: Probably should just use
        //  static_vector inside the functions.

        std::vector<VkRenderingAttachmentInfoKHR> color_attachments_;
        std::vector<VkViewport> viewports_;
        std::vector<VkRect2D> scissors_;
        std::vector<VkBuffer> vertex_buffers_;
        std::vector<VkDeviceSize> vertex_buffer_offsets_;
    };

    class VulkanCommandAllocator final : public CommandAllocator
    {
    public:
        VulkanCommandAllocator(VkDevice device, VkCommandPool command_pool);
        ~VulkanCommandAllocator() override;

        [[nodiscard]] VkCommandPool vk_command_pool() const { return command_pool_; }

    private:
        void reset_api() override;

        VkDevice device_;
        VkCommandPool command_pool_;
    };
} // namespace orion
