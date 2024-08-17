#pragma once

#include "orion/renderapi/render_backend.h"

#include "orion_dx12.h"

#include <vector>

namespace orion
{
    class D3D12Backend final : public RenderBackend
    {
    public:
        D3D12Backend();

        [[nodiscard]] const char* name() const noexcept override { return "DirectX12"; }

    private:
        std::vector<GraphicsAdapter> get_adapters_api() override;
        std::unique_ptr<RenderDevice> create_device_api(std::size_t adapter_index) override;

        ComPtr<ID3D12Debug> debug_controller_;
        ComPtr<IDXGIFactory2> factory_;
        std::vector<ComPtr<IDXGIAdapter1>> adapters_;
    };
} // namespace orion
