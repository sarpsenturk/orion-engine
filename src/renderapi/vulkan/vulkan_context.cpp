#include "vulkan_context.hpp"

namespace orion
{
    VulkanContext::VulkanContext(VkDevice device, VmaAllocator allocator)
        : device_(device)
        , allocator_(allocator)
    {
    }

    BindGroupLayoutHandle VulkanContext::insert(VulkanDescriptorSetLayout descriptor_set_layout)
    {
        return BindGroupLayoutHandle{descriptor_set_layouts_.insert(UniqueVulkanDescriptorSetLayout{descriptor_set_layout, {device_}})};
    }

    PipelineLayoutHandle VulkanContext::insert(VkPipelineLayout pipeline_layout)
    {
        return PipelineLayoutHandle{pipeline_layouts_.insert(UniqueVkPipelineLayout{pipeline_layout, {device_}})};
    }

    PipelineHandle VulkanContext::insert(VkPipeline pipeline)
    {
        return PipelineHandle{pipelines_.insert(UniqueVkPipeline{pipeline, {device_}})};
    }

    BufferHandle VulkanContext::insert(VulkanBuffer buffer)
    {
        return BufferHandle{buffers_.insert(UniqueVulkanBuffer(buffer, {allocator_}))};
    }

    ImageHandle VulkanContext::insert(VulkanImage image, VulkanImageData data)
    {
        return ImageHandle{images_.insert(UniqueVulkanImage(image, {allocator_}), data)};
    }

    ImageViewHandle VulkanContext::insert(VkImageView image_view)
    {
        return ImageViewHandle{image_views_.insert(UniqueVkImageView{image_view, {device_}})};
    }

    SemaphoreHandle VulkanContext::insert(VkSemaphore semaphore)
    {
        return SemaphoreHandle{semaphores_.insert(UniqueVkSemaphore{semaphore, {device_}})};
    }

    FenceHandle VulkanContext::insert(VkFence fence)
    {
        return FenceHandle{fences_.insert(UniqueVkFence{fence, {device_}})};
    }

    BindGroupHandle VulkanContext::insert(VkDescriptorSet descriptor_set, VkDescriptorPool descriptor_pool)
    {
        return BindGroupHandle{descriptor_sets_.insert(UniqueVkDescriptorSet{descriptor_set, {device_, descriptor_pool}})};
    }

    SamplerHandle VulkanContext::insert(VkSampler sampler)
    {
        return SamplerHandle{samplers_.insert(UniqueVkSampler{sampler, {device_}})};
    }

    VulkanDescriptorSetLayout VulkanContext::lookup(BindGroupLayoutHandle bind_group_layout) const
    {
        return descriptor_set_layouts_.lookup(static_cast<render_device_handle_t>(bind_group_layout));
    }

    VkPipelineLayout VulkanContext::lookup(PipelineLayoutHandle pipeline_layout) const
    {
        return pipeline_layouts_.lookup(static_cast<render_device_handle_t>(pipeline_layout));
    }

    VkPipeline VulkanContext::lookup(PipelineHandle pipeline) const
    {
        return pipelines_.lookup(static_cast<render_device_handle_t>(pipeline));
    }

    VulkanBuffer VulkanContext::lookup(BufferHandle buffer) const
    {
        return buffers_.lookup(static_cast<render_device_handle_t>(buffer));
    }

    VulkanImage VulkanContext::lookup(ImageHandle image) const
    {
        return images_.lookup(static_cast<render_device_handle_t>(image));
    }

    VkImageView VulkanContext::lookup(ImageViewHandle image_view) const
    {
        return image_views_.lookup(static_cast<render_device_handle_t>(image_view));
    }

    VkSemaphore VulkanContext::lookup(SemaphoreHandle semaphore) const
    {
        return semaphores_.lookup(static_cast<render_device_handle_t>(semaphore));
    }

    VkFence VulkanContext::lookup(FenceHandle fence) const
    {
        return fences_.lookup(static_cast<render_device_handle_t>(fence));
    }

    VkDescriptorSet VulkanContext::lookup(BindGroupHandle descriptor_set) const
    {
        return descriptor_sets_.lookup(static_cast<render_device_handle_t>(descriptor_set));
    }

    VkSampler VulkanContext::lookup(SamplerHandle sampler) const
    {
        return samplers_.lookup(static_cast<render_device_handle_t>(sampler));
    }

    VulkanImageData* VulkanContext::lookup_data(ImageHandle image)
    {
        return images_.lookup_data(static_cast<render_device_handle_t>(image));
    }

    bool VulkanContext::remove(BindGroupLayoutHandle bind_group_layout)
    {
        return descriptor_set_layouts_.remove(static_cast<render_device_handle_t>(bind_group_layout));
    }

    bool VulkanContext::remove(PipelineLayoutHandle pipeline_layout)
    {
        return pipeline_layouts_.remove(static_cast<render_device_handle_t>(pipeline_layout));
    }

    bool VulkanContext::remove(PipelineHandle pipeline)
    {
        return pipelines_.remove(static_cast<render_device_handle_t>(pipeline));
    }

    bool VulkanContext::remove(BufferHandle buffer)
    {
        return buffers_.remove(static_cast<render_device_handle_t>(buffer));
    }

    bool VulkanContext::remove(ImageHandle image)
    {
        return images_.remove(static_cast<render_device_handle_t>(image));
    }

    bool VulkanContext::remove(ImageViewHandle render_target)
    {
        return image_views_.remove(static_cast<render_device_handle_t>(render_target));
    }

    bool VulkanContext::remove(SemaphoreHandle semaphore)
    {
        return semaphores_.remove(static_cast<render_device_handle_t>(semaphore));
    }

    bool VulkanContext::remove(FenceHandle fence)
    {
        return fences_.remove(static_cast<render_device_handle_t>(fence));
    }

    bool VulkanContext::remove(BindGroupHandle descriptor_set)
    {
        return descriptor_sets_.remove(static_cast<render_device_handle_t>(descriptor_set));
    }

    bool VulkanContext::remove(SamplerHandle sampler)
    {
        return samplers_.remove(static_cast<render_device_handle_t>(sampler));
    }
} // namespace orion
