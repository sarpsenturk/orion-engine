#include "orion/renderer/renderer.hpp"

#include "orion/rhi/rhi_instance.hpp"

#include "orion/assert.hpp"
#include "orion/log.hpp"

namespace orion
{
    namespace
    {
        std::unique_ptr<RHIInstance> rhi;
        std::unique_ptr<RHIDevice> rhi_device;
        std::unique_ptr<RHICommandQueue> rhi_command_queue;
    } // namespace

    bool Renderer::init()
    {
        ORION_ASSERT(rhi == nullptr, "Renderer has already been initialized");
        if (rhi = rhi_create_instance(); rhi == nullptr) {
            ORION_CORE_LOG_ERROR("Failed to create RHIInstance");
            return false;
        }
        if (rhi_device = rhi->create_device(); rhi == nullptr) {
            ORION_CORE_LOG_ERROR("Failed to create RHIDevice");
            return false;
        }
        if (rhi_command_queue = rhi_device->create_command_queue({.type = RHICommandQueueType::Graphics}); rhi_command_queue == nullptr) {
            ORION_CORE_LOG_ERROR("Failed to create RHICommandQueue");
            return false;
        }
        return true;
    }

    void Renderer::shutdown()
    {
        ORION_ASSERT(rhi != nullptr, "Renderer has not been initialized or has already been shut down");
        rhi_command_queue = nullptr;
        rhi_device = nullptr;
        rhi = nullptr;
    }
} // namespace orion
