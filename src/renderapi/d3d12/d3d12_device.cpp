#include "d3d12_device.h"

namespace orion
{
    D3D12Device::D3D12Device(ComPtr<ID3D12Device> device)
        : device_(std::move(device))
    {
    }
} // namespace orion
