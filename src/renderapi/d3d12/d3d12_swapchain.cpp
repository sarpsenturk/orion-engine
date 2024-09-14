#include "d3d12_swapchain.h"

#include <spdlog/spdlog.h>

#include <utility>

namespace orion
{
    D3D12Swapchain::D3D12Swapchain(ID3D12Device* device, ComPtr<IDXGISwapChain1> swapchain, D3D12Context* context)
        : device_(device)
        , swapchain_(std::move(swapchain))
        , context_(context)
    {
        // Get buffer count & format from swapchain desc
        const auto [buffer_count, buffer_format] = [&]() {
            DXGI_SWAP_CHAIN_DESC desc;
            hr_assert(swapchain_->GetDesc(&desc));
            return std::make_pair(desc.BufferCount, desc.BufferDesc.Format);
        }();

        // Create RTV heap
        ComPtr<ID3D12DescriptorHeap> rtv_heap;
        {
            const auto desc = D3D12_DESCRIPTOR_HEAP_DESC{
                .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
                .NumDescriptors = buffer_count,
                .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
                .NodeMask = 0,
            };
            hr_assert(device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtv_heap)));
            SPDLOG_TRACE("Create ID3D12DescrpitorHeap (D3D12_DESCRIPTOR_HEAP_TYPE_RTV) {} for swapchain RTVs", fmt::ptr(rtv_heap.Get()));
        }

        // Get buffers & create RTVs
        auto rtv_handle = rtv_heap->GetCPUDescriptorHandleForHeapStart();
        const auto increment = device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        for (UINT i = 0; i < buffer_count; ++i) {
            ComPtr<ID3D12Resource> back_buffer;
            hr_assert(swapchain_->GetBuffer(i, IID_PPV_ARGS(&back_buffer)));
            SPDLOG_TRACE("Got ID3D12Resource (back buffer) {} from swapchain", fmt::ptr(back_buffer.Get()));
            back_buffers_.push_back(context_->insert_image(back_buffer));
            device_->CreateRenderTargetView(back_buffer.Get(), nullptr, rtv_handle);
            SPDLOG_TRACE("Created RTV {:#x}", rtv_handle.ptr);
            render_targets_.push_back(context_->insert_render_target(rtv_heap, rtv_handle));
            rtv_handle.ptr += increment;
        }
    }

    D3D12Swapchain::~D3D12Swapchain()
    {
        for (const auto render_target : render_targets_) {
            context_->remove_render_target(render_target);
        }
        for (const auto back_buffer : back_buffers_) {
            context_->remove_image(back_buffer);
        }
    }

    RenderTargetHandle D3D12Swapchain::acquire_render_target_api()
    {
        return render_targets_[image_index_];
    }

    ImageHandle D3D12Swapchain::current_image_api()
    {
        return back_buffers_[image_index_];
    }

    void D3D12Swapchain::present_api(bool vsync)
    {
        hr_assert(swapchain_->Present(vsync, 0));
    }
} // namespace orion
