#pragma once

#include "orion/renderapi/swapchain.h"

#include "orion_dx12.h"

#include "d3d12_context.h"

#include <vector>

namespace orion
{
    class D3D12Swapchain final : public Swapchain
    {
    public:
        D3D12Swapchain(ID3D12Device* device, ComPtr<IDXGISwapChain1> swapchain, D3D12Context* context);
        ~D3D12Swapchain() override;

    private:
        RenderTargetHandle acquire_render_target_api() override;
        ImageHandle current_image_api() override;
        void present_api(bool vsync) override;

        ID3D12Device* device_;
        ComPtr<IDXGISwapChain1> swapchain_;
        D3D12Context* context_;

        std::vector<ImageHandle> back_buffers_;
        std::vector<RenderTargetHandle> render_targets_;
        std::uint32_t image_index_ = 0;
    };
} // namespace orion
