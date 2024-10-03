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
        for (const auto [fence, _] : fences_) {
            vkDestroyFence(device_, fence, nullptr);
        }

        for (const auto [semaphore, _] : semaphores_) {
            vkDestroySemaphore(device_, semaphore, nullptr);
        }

        for (const auto [image_view, _] : image_views_) {
            vkDestroyImageView(device_, image_view, nullptr);
        }

        // TODO: Destroy user created images

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

    ImageHandle VulkanContext::insert(VkImage image)
    {
        return ImageHandle{insert(images_, image)};
    }

    RenderTargetHandle VulkanContext::insert(VkImageView image_view)
    {
        return RenderTargetHandle{insert(image_views_, image_view)};
    }

    SemaphoreHandle VulkanContext::insert(VkSemaphore semaphore)
    {
        return SemaphoreHandle{insert(semaphores_, semaphore)};
    }

    FenceHandle VulkanContext::insert(VkFence fence)
    {
        return FenceHandle{insert(fences_, fence)};
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

    VkImage VulkanContext::lookup(ImageHandle image) const
    {
        return lookup(images_, static_cast<render_device_handle_t>(image));
    }

    VkImageView VulkanContext::lookup(RenderTargetHandle render_target) const
    {
        return lookup(image_views_, static_cast<render_device_handle_t>(render_target));
    }

    VkSemaphore VulkanContext::lookup(SemaphoreHandle semaphore) const
    {
        return lookup(semaphores_, static_cast<render_device_handle_t>(semaphore));
    }

    VkFence VulkanContext::lookup(FenceHandle fence) const
    {
        return lookup(fences_, static_cast<render_device_handle_t>(fence));
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

    bool VulkanContext::remove(ImageHandle image)
    {
        return remove(images_, static_cast<render_device_handle_t>(image), [](VkImage) {});
    }

    bool VulkanContext::remove(RenderTargetHandle render_target)
    {
        auto deleter = [this](VkImageView image_view) {
            vkDestroyImageView(device_, image_view, nullptr);
        };
        return remove(image_views_, static_cast<render_device_handle_t>(render_target), deleter);
    }

    bool VulkanContext::remove(SemaphoreHandle semaphore)
    {
        auto deleter = [this](VkSemaphore semaphore) {
            vkDestroySemaphore(device_, semaphore, nullptr);
        };
        return remove(semaphores_, static_cast<render_device_handle_t>(semaphore), deleter);
    }

    bool VulkanContext::remove(FenceHandle fence)
    {
        auto deleter = [this](VkFence fence) {
            vkDestroyFence(device_, fence, nullptr);
        };
        return remove(fences_, static_cast<render_device_handle_t>(fence), deleter);
    }
} // namespace orion
