#pragma once

#include "vulkan_headers.h"
#include "vulkan_types.h"

namespace orion::vulkan
{
    class VulkanPipeline
    {
    public:
        VulkanPipeline(UniqueVkPipelineLayout pipeline_layout, UniqueVkPipeline pipeline);

    private:
        UniqueVkPipelineLayout pipeline_layout_;
        UniqueVkPipeline pipeline_;
    };
} // namespace orion::vulkan
