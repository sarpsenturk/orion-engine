#pragma once

#include "orion/renderapi/render_device.h"

#include "orion_dx12.h"

namespace orion
{
    class D3D12Device final : public RenderDevice
    {
    public:
        D3D12Device(ComPtr<ID3D12Device> device);

    private:
        ComPtr<ID3D12Device> device_;
    };
} // namespace orion
