#pragma once

#include "orion/renderapi/swapchain.h"

#include "orion_dx12.h"

namespace orion
{
    class D3D12Swapchain final : public Swapchain
    {
    public:
        D3D12Swapchain(ComPtr<IDXGISwapChain1> swapchain);

    private:
        ComPtr<IDXGISwapChain1> swapchain_;
    };
} // namespace orion
