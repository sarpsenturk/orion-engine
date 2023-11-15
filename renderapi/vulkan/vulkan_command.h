#pragma once

#include "orion-renderapi/render_command.h"

#include "vulkan_headers.h"
#include "vulkan_types.h"

namespace orion::vulkan
{
    class VulkanDevice;

    class VulkanCommandList final : public CommandList
    {
    public:
        VulkanCommandList(VulkanDevice* device, UniqueVkCommandBuffer command_buffer);

    private:
        void begin_api() override;
        void end_api() override;
        void draw_api(const CmdDraw& cmd_draw) override;
        void draw_indexed_api(const CmdDrawIndexed& cmd_draw_indexed) override;

        VulkanDevice* device_;
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
