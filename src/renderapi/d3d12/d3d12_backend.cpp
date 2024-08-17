#include "d3d12_backend.h"

#include <spdlog/spdlog.h>

namespace orion
{
    D3D12Backend::D3D12Backend()
    {
        // Enable debug layer
#ifdef ORION_BUILD_DEBUG
        hr_assert(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller_)));
        debug_controller_->EnableDebugLayer();
        SPDLOG_TRACE("Acquired ID3D12Debug interface at {}", fmt::ptr(debug_controller_.Get()));
#endif

        // Create DXGI factory
        hr_assert(CreateDXGIFactory1(IID_PPV_ARGS(&factory_)));
        SPDLOG_TRACE("Created IDXGIFactory2 interface at {}", fmt::ptr(factory_.Get()));

        SPDLOG_DEBUG("DirectX12 backend initialized");
    }
} // namespace orion
