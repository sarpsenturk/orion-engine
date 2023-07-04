#include "vulkan_pipeline.h"

namespace orion::vulkan
{
    VulkanPipeline::VulkanPipeline(UniqueVkDescriptorSetLayout descriptor_set_layout,
                                   UniqueVkPipelineLayout pipeline_layout,
                                   UniqueVkPipeline pipeline)
        : descriptor_set_layout_(std::move(descriptor_set_layout))
        , pipeline_layout_(std::move(pipeline_layout))
        , pipeline_(std::move(pipeline))
    {
    }
} // namespace orion::vulkan
