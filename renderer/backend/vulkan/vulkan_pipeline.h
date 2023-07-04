#pragma once

#include "vulkan_headers.h"
#include "vulkan_types.h"

namespace orion::vulkan
{
    class VulkanPipeline
    {
    public:
        VulkanPipeline(UniqueVkDescriptorSetLayout descriptor_set_layout,
                       UniqueVkPipelineLayout pipeline_layout,
                       UniqueVkPipeline pipeline);

        [[nodiscard]] auto pipeline() const noexcept { return pipeline_.get(); }
        [[nodiscard]] auto pipeline_layout() const noexcept { return pipeline_layout_.get(); }

    private:
        UniqueVkDescriptorSetLayout descriptor_set_layout_;
        UniqueVkPipelineLayout pipeline_layout_;
        UniqueVkPipeline pipeline_;
    };
} // namespace orion::vulkan
