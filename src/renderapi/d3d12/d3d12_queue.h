#pragma once

#include "orion/renderapi/render_queue.h"

#include "orion_dx12.h"

namespace orion
{
    class D3D12Queue final : public CommandQueue
    {
    public:
        D3D12Queue(ComPtr<ID3D12CommandQueue> queue);

    private:
        ComPtr<ID3D12CommandQueue> queue_;
    };
} // namespace orion
