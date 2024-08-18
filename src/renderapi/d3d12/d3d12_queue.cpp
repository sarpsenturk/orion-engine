#include "d3d12_queue.h"

namespace orion
{
    D3D12Queue::D3D12Queue(ComPtr<ID3D12CommandQueue> queue)
        : queue_(std::move(queue))
    {
    }
} // namespace orion
