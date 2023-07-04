#include "vulkan_pipeline.h"

namespace orion::vulkan
{
    VulkanPipeline::VulkanPipeline(UniqueVkPipelineLayout pipeline_layout,
                                   UniqueVkPipeline pipeline)
        : pipeline_layout_(std::move(pipeline_layout))
        , pipeline_(std::move(pipeline))
    {
    }
} // namespace orion::vulkan
