#pragma once

#include "orion/renderapi/render_command.h"

#include <Volk/volk.h>

namespace orion
{
    class VulkanCommandList final : public CommandList
    {
    public:
        VulkanCommandList(VkDevice device, VkCommandBuffer command_buffer);

    private:
        void begin_api() override;
        void end_api() override;

        VkDevice device_;
        VkCommandBuffer command_buffer_;
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
