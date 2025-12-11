#include "orion/rhi/rhi_instance.hpp"

namespace orion
{
    extern std::unique_ptr<RHIInstance> rhi_vulkan_create_instance();

    std::unique_ptr<RHIInstance> rhi_create_instance()
    {
        return rhi_vulkan_create_instance();
    }

    std::unique_ptr<RHIDevice> RHIInstance::create_device()
    {
        return create_device_api();
    }
} // namespace orion
