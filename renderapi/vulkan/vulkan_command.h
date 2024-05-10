#pragma once

#include "orion-renderapi/render_command.h"

#include "vulkan_headers.h"
#include "vulkan_types.h"

namespace orion::vulkan
{
    class VulkanDevice;
    class VulkanResourceManager;

    class VulkanCommandList final : public CommandList
    {
    public:
        VulkanCommandList(VulkanResourceManager* resource_manager, UniqueVkCommandBuffer command_buffer);

        [[nodiscard]] VkCommandBuffer vk_command_buffer() const noexcept { return command_buffer_.get(); }

    private:
        void begin_api() override;
        void end_api() override;
        void reset_api() override;
        void draw_api(const CmdDraw& cmd_draw) override;
        void draw_indexed_api(const CmdDrawIndexed& cmd_draw_indexed) override;
        void bind_index_buffer_api(const CmdBindIndexBuffer& cmd_bind_index_buffer) override;
        void bind_vertex_buffer_api(const CmdBindVertexBuffer& cmd_bind_vertex_buffer) override;
        void bind_pipeline_api(const CmdBindPipeline& cmd_bind_pipeline) override;
        void bind_descriptor_api(const CmdBindDescriptor& cmd_bind_descriptor) override;
        void begin_render_pass_api(const CmdBeginRenderPass& cmd_begin_render_pass) override;
        void end_render_pass_api() override;
        void set_viewports_api(const CmdSetViewports& cmd_set_viewports) override;
        void set_scissors_api(const CmdSetScissors& cmd_set_scissors) override;
        void push_constants_api(const CmdPushConstants& cmd_push_constants) override;
        void copy_buffer_api(const CmdCopyBuffer& cmd_copy_buffer) override;
        void copy_buffer_to_image_api(const CmdCopyBufferToImage& cmd_copy_buffer_to_image) override;
        void transition_barrier_api(const CmdTransitionBarrier& cmd_transition_barrier) override;

        VulkanResourceManager* resource_manager_;
        UniqueVkCommandBuffer command_buffer_;
    };

    class VulkanCommandAllocator final : public CommandAllocator
    {
    public:
        VulkanCommandAllocator(VulkanDevice* device, UniqueVkCommandPool command_pool);

    private:
        void reset_api() override;
        std::unique_ptr<CommandList> create_command_list_api() override;

        VulkanDevice* device_;
        UniqueVkCommandPool command_pool_;
    };
} // namespace orion::vulkan
