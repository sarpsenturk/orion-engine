#include "d3d12_device.h"

#include "d3d12_queue.h"
#include "d3d12_swapchain.h"

#include "win32/win32_window.h"

#include <spdlog/spdlog.h>

namespace orion
{
    D3D12Device::D3D12Device(ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory2> factory)
        : device_(std::move(device))
        , factory_(std::move(factory))
    {
    }

    std::unique_ptr<CommandQueue> D3D12Device::create_command_queue_api()
    {
        const auto desc = D3D12_COMMAND_QUEUE_DESC{
            .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
            .Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
            .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
            .NodeMask = 0,
        };
        ComPtr<ID3D12CommandQueue> queue;
        hr_assert(device_->CreateCommandQueue(&desc, IID_PPV_ARGS(&queue)));
        SPDLOG_TRACE("Created ID3D12CommandQueue interface at {}", fmt::ptr(queue.Get()));
        return std::make_unique<D3D12Queue>(queue);
    }

    std::unique_ptr<Swapchain> D3D12Device::create_swapchain_api(const SwapchainDesc& desc)
    {
        const auto dxgi_desc = DXGI_SWAP_CHAIN_DESC1{
            .Width = desc.width,
            .Height = desc.height,
            .Format = DXGI_FORMAT_B8G8R8A8_UNORM,
            .Stereo = FALSE,
            .SampleDesc = {.Count = 1, .Quality = 0},
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = desc.image_count,
            .Scaling = DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
            .Flags = 0,
        };

        ID3D12CommandQueue* queue = static_cast<const D3D12Queue*>(desc.queue)->get();
        HWND hwnd = desc.window->platform_window()->hwnd;

        ComPtr<IDXGISwapChain1> swapchain;
        hr_assert(factory_->CreateSwapChainForHwnd(queue, hwnd, &dxgi_desc, nullptr, nullptr, &swapchain));
        SPDLOG_TRACE("Created IDXGISwapChain1 interface at {}", fmt::ptr(swapchain.Get()));

        return std::make_unique<D3D12Swapchain>(std::move(swapchain));
    }
} // namespace orion
