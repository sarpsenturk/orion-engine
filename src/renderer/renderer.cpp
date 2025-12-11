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
        return true;
    }

    void Renderer::shutdown()
    {
        ORION_ASSERT(rhi != nullptr, "Renderer has not been initialized or has already been shut down");
        rhi_device = nullptr;
        rhi = nullptr;
    }
} // namespace orion
