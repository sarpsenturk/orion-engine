#include "d3d12_command.h"

namespace orion
{
    D3D12CommandList::D3D12CommandList(ComPtr<ID3D12GraphicsCommandList> command_list)
        : command_list_(std::move(command_list))
    {
    }

    void D3D12CommandList::begin_api()
    {
    }

    void D3D12CommandList::end_api()
    {
    }

    D3D12CommandAllocator::D3D12CommandAllocator(ComPtr<ID3D12CommandAllocator> command_allocator)
        : command_allocator_(std::move(command_allocator))
    {
    }

    void D3D12CommandAllocator::reset_api()
    {
    }
} // namespace orion
