#include "vulkan_context.h"

namespace orion
{
    VulkanContext::VulkanContext(VkDevice device, VmaAllocator allocator)
        : device_(device)
        , allocator_(allocator)
    {
    }

    VulkanContext::~VulkanContext()
    {
        for (const auto [buffer, _] : buffers_) {
            vmaDestroyBuffer(allocator_, buffer.buffer, buffer.allocation);
        }

        for (const auto [pipeline, _] : pipelines_) {
            vkDestroyPipeline(device_, pipeline, nullptr);
        }

        for (const auto [pipeline_layout, _] : pipeline_layouts_) {
            vkDestroyPipelineLayout(device_, pipeline_layout, nullptr);
        }

        vmaDestroyAllocator(allocator_);

        vkDestroyDevice(device_, nullptr);
    }

    PipelineLayoutHandle VulkanContext::insert(VkPipelineLayout pipeline_layout)
    {
        return PipelineLayoutHandle{insert(pipeline_layouts_, pipeline_layout)};
    }

    PipelineHandle VulkanContext::insert(VkPipeline pipeline)
    {
        return PipelineHandle{insert(pipelines_, pipeline)};
    }

    BufferHandle VulkanContext::insert(VkBuffer buffer, VmaAllocation allocation)
    {
        return BufferHandle{insert(buffers_, VulkanBuffer{.buffer = buffer, .allocation = allocation})};
    }

    VkPipelineLayout VulkanContext::lookup(PipelineLayoutHandle pipeline_layout) const
    {
        return lookup(pipeline_layouts_, static_cast<render_device_handle_t>(pipeline_layout));
    }

    VkPipeline VulkanContext::lookup(PipelineHandle pipeline) const
    {
        return lookup(pipelines_, static_cast<render_device_handle_t>(pipeline));
    }

    VulkanBuffer VulkanContext::lookup(BufferHandle buffer) const
    {
        return lookup(buffers_, static_cast<render_device_handle_t>(buffer));
    }

    bool VulkanContext::remove(PipelineLayoutHandle pipeline_layout)
    {
        auto deleter = [this](VkPipelineLayout pipeline_layout) {
            vkDestroyPipelineLayout(device_, pipeline_layout, nullptr);
        };
        return remove(pipeline_layouts_, static_cast<render_device_handle_t>(pipeline_layout), deleter);
    }

    bool VulkanContext::remove(PipelineHandle pipeline)
    {
        auto deleter = [this](VkPipeline pipeline) {
            vkDestroyPipeline(device_, pipeline, nullptr);
        };
        return remove(pipelines_, static_cast<render_device_handle_t>(pipeline), deleter);
    }

    bool VulkanContext::remove(BufferHandle buffer)
    {
        auto deleter = [this](VulkanBuffer buffer) {
            vmaDestroyBuffer(allocator_, buffer.buffer, buffer.allocation);
        };
        return remove(buffers_, static_cast<render_device_handle_t>(buffer), deleter);
    }
} // namespace orion
