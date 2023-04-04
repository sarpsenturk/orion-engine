#pragma once

#include "orion-renderapi/render_context.h"
#include "vulkan_headers.h"

#include <vector> // std::vector

namespace orion::vulkan
{
    class VulkanRenderContext : public RenderContext
    {
    public:
        explicit VulkanRenderContext(UniqueVkCommandPool command_pool);

    private:
        VkCommandBuffer allocate_command_buffer(VkCommandBufferLevel level);
        std::vector<VkCommandBuffer> allocate_command_buffers(std::uint32_t count, VkCommandBufferLevel level);

        VkDevice device_;
        UniqueVkCommandPool command_pool_;
        VkCommandBuffer command_buffer_;
    };
} // namespace orion::vulkan
