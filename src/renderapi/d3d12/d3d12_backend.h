#pragma once

#include "orion/renderapi/render_backend.h"

#include "orion_dx12.h"

namespace orion
{
    class D3D12Backend final : public RenderBackend
    {
    public:
        D3D12Backend();

        [[nodiscard]] const char* name() const noexcept override { return "DirectX12"; }

    private:
        ComPtr<ID3D12Debug> debug_controller_;
        ComPtr<IDXGIFactory2> factory_;
    };
} // namespace orion
