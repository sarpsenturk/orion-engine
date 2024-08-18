#include "d3d12_device.h"

#include "d3d12_queue.h"

#include <spdlog/spdlog.h>

namespace orion
{
    D3D12Device::D3D12Device(ComPtr<ID3D12Device> device)
        : device_(std::move(device))
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
} // namespace orion
