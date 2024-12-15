#include "vulkan_descriptor.hpp"

namespace orion
{
    VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VkDescriptorSetLayout _layout, VkDescriptorPool _pool)
        : layout(_layout)
        , pool(_pool)
    {
    }
} // namespace orion
