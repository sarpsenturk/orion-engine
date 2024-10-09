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
        // Don't need to call vkFreeDescriptorSets as vkDestroyDescriptorPool will handle it
        for (const auto [descriptor_pool, _] : descriptor_pools_) {
            vkDestroyDescriptorPool(device_, descriptor_pool, nullptr);
        }

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

        for (const auto [descriptor_set_layout, _] : descriptor_set_layouts_) {
            vkDestroyDescriptorSetLayout(device_, descriptor_set_layout, nullptr);
        }

        vmaDestroyAllocator(allocator_);

        vkDestroyDevice(device_, nullptr);
    }

    DescriptorSetLayoutHandle VulkanContext::insert(VkDescriptorSetLayout descriptor_set_layout)
    {
        return DescriptorSetLayoutHandle{insert(descriptor_set_layouts_, descriptor_set_layout)};
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

    DescriptorPoolHandle VulkanContext::insert(VkDescriptorPool descriptor_pool)
    {
        return DescriptorPoolHandle{insert(descriptor_pools_, descriptor_pool)};
    }

    DescriptorSetHandle VulkanContext::insert(VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool)
    {
        return DescriptorSetHandle{insert(descriptor_sets_, VulkanDescriptorSet{descriptor_set, descriptor_pool})};
    }

    VkDescriptorSetLayout VulkanContext::lookup(DescriptorSetLayoutHandle descriptor_set_layout) const
    {
        return lookup(descriptor_set_layouts_, static_cast<render_device_handle_t>(descriptor_set_layout));
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

    VkDescriptorPool VulkanContext::lookup(DescriptorPoolHandle descriptor_pool) const
    {
        return lookup(descriptor_pools_, static_cast<render_device_handle_t>(descriptor_pool));
    }

    VkDescriptorSet VulkanContext::lookup(DescriptorSetHandle descriptor_set) const
    {
        return lookup(descriptor_sets_, static_cast<render_device_handle_t>(descriptor_set)).set;
    }

    bool VulkanContext::remove(DescriptorSetLayoutHandle descriptor_set_layout)
    {
        auto deleter = [this](VkDescriptorSetLayout descriptor_set_layout) {
            vkDestroyDescriptorSetLayout(device_, descriptor_set_layout, nullptr);
        };
        return remove(descriptor_set_layouts_, static_cast<render_device_handle_t>(descriptor_set_layout), deleter);
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

    bool VulkanContext::remove(DescriptorPoolHandle descriptor_pool)
    {
        auto deleter = [this](VkDescriptorPool descriptor_pool) {
            vkDestroyDescriptorPool(device_, descriptor_pool, nullptr);
        };
        return remove(descriptor_pools_, static_cast<render_device_handle_t>(descriptor_pool), deleter);
    }

    bool VulkanContext::remove(DescriptorSetHandle descriptor_set)
    {
        auto deleter = [this](VulkanDescriptorSet descriptor_set) {
            vkFreeDescriptorSets(device_, descriptor_set.pool, 1, &descriptor_set.set);
        };
        return remove(descriptor_sets_, static_cast<render_device_handle_t>(descriptor_set), deleter);
    }
} // namespace orion
