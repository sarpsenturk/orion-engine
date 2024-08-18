#include "d3d12_swapchain.h"

namespace orion
{
    D3D12Swapchain::D3D12Swapchain(ComPtr<IDXGISwapChain1> swapchain)
        : swapchain_(std::move(swapchain))
    {
    }
} // namespace orion
