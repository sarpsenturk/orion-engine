#include "orion/rhi/rhi_instance.hpp"

namespace orion
{
    extern RHIInstance* rhi_vulkan_create_instance();
    extern void rhi_vulkan_destroy_instance(RHIInstance* instance);

    RHIInstance* rhi_create_instance()
    {
        return rhi_vulkan_create_instance();
    }

    void rhi_destroy_instance(RHIInstance* instance)
    {
        rhi_vulkan_destroy_instance(instance);
    }
} // namespace orion
