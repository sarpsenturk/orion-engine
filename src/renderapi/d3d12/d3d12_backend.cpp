#include "d3d12_backend.h"

#include "d3d12_device.h"

#include "orion/assertion.h"

#include <spdlog/spdlog.h>

#include <algorithm>

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

        // Get adapters
        get_adapters_api();

        SPDLOG_DEBUG("DirectX12 backend initialized");
    }

    std::vector<GraphicsAdapter> D3D12Backend::get_adapters_api()
    {
        // Get the list of current adapters
        {
            adapters_.clear();
            UINT i = 0;
            while (true) {
                ComPtr<IDXGIAdapter1> adapter;
                if (HRESULT hr = factory_->EnumAdapters1(i++, &adapter); hr != S_OK) {
                    break;
                }
                adapters_.emplace_back(std::move(adapter));
            }
        }

        std::vector<GraphicsAdapter> adapters(adapters_.size());
        std::ranges::transform(adapters_, adapters.begin(), [](const ComPtr<IDXGIAdapter1>& adapter) {
            DXGI_ADAPTER_DESC desc;
            hr_assert(adapter->GetDesc(&desc));
            return GraphicsAdapter{.name = wstring_to_string(desc.Description)};
        });
        return adapters;
    }

    std::unique_ptr<RenderDevice> D3D12Backend::create_device_api(std::size_t adapter_index)
    {
        ORION_EXPECTS(adapter_index < adapters_.size());
        auto adapter = adapters_[adapter_index];
        SPDLOG_TRACE("Creating ID3D12Device with IDXGIAdapter1 interface {}", fmt::ptr(adapter.Get()));

        ComPtr<ID3D12Device> device;
        hr_assert(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device)));
        SPDLOG_TRACE("Created ID3D12Device interface at {}", fmt::ptr(device.Get()));
        return std::make_unique<D3D12Device>(std::move(device), factory_, std::move(adapter));
    }
} // namespace orion
