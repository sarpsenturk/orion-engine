#include "orion/renderer/renderer.hpp"

#include "orion/rhi/rhi_instance.hpp"

#include "orion/assert.hpp"
#include "orion/log.hpp"

namespace orion
{
    namespace
    {
        RHIInstance* rhi;
    }

    bool Renderer::init()
    {
        ORION_ASSERT(rhi == nullptr, "Renderer has already been initialized");
        if (rhi = rhi_create_instance(); rhi == nullptr) {
            ORION_CORE_LOG_ERROR("Failed to create RHIInstance");
            return false;
        }
        return true;
    }

    void Renderer::shutdown()
    {
        ORION_ASSERT(rhi != nullptr, "Renderer has not been initialized or has already been shut down");
        rhi_destroy_instance(rhi);
        rhi = nullptr;
    }
} // namespace orion
