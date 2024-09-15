#pragma once

#include "orion/renderapi/render_queue.h"

#include "d3d12_context.h"
#include "orion_dx12.h"

#include <vector>

namespace orion
{
    class D3D12Queue final : public CommandQueue
    {
    public:
        D3D12Queue(ComPtr<ID3D12CommandQueue> queue, D3D12Context* context);

        [[nodiscard]] ID3D12CommandQueue* get() const { return queue_.Get(); }

    private:
        void wait_api(SemaphoreHandle semaphore) override;
        void signal_api(SemaphoreHandle semaphore) override;
        void submit_api(std::span<const class CommandList* const> command_lists, FenceHandle fence) override;

        ComPtr<ID3D12CommandQueue> queue_;
        D3D12Context* context_;

        // Command lists stored in object to avoid reallocation on every submit_api call
        std::vector<ID3D12CommandList*> command_lists_;
        std::vector<ComPtr<ID3D12Fence>> wait_fences_;
        std::vector<ComPtr<ID3D12Fence>> signal_fences_;
    };
} // namespace orion
