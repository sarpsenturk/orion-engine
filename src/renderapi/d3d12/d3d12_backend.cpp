#include "d3d12_backend.h"

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

        SPDLOG_DEBUG("DirectX12 backend initialized");
    }

    std::vector<GraphicsAdapter> D3D12Backend::get_adapters_api()
    {
        // Get the list of current adapters
        {
            adapters_.clear();
            UINT i = 0;
            while (true) {
                ComPtr<IDXGIAdapter> adapter;
                if (HRESULT hr = factory_->EnumAdapters(i++, &adapter); hr != S_OK) {
                    break;
                }
                adapters_.emplace_back(std::move(adapter));
            }
        }

        std::vector<GraphicsAdapter> adapters(adapters_.size());
        std::ranges::transform(adapters_, adapters.begin(), [](const ComPtr<IDXGIAdapter>& adapter) {
            DXGI_ADAPTER_DESC desc;
            hr_assert(adapter->GetDesc(&desc));
            return GraphicsAdapter{.name = wstring_to_string(desc.Description)};
        });
        return adapters;
    }
} // namespace orion
