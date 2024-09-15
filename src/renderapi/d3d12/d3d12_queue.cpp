#include "d3d12_queue.h"

#include "d3d12_command.h"

#include "orion/assertion.h"

#include <algorithm>

namespace orion
{
    D3D12Queue::D3D12Queue(ComPtr<ID3D12CommandQueue> queue, D3D12Context* context)
        : queue_(std::move(queue))
        , context_(context)
    {
    }

    void D3D12Queue::wait_api(SemaphoreHandle semaphore)
    {
        ComPtr<ID3D12Fence> dx_fence = context_->get_semaphore(semaphore);
        ORION_ASSERT(dx_fence != nullptr);
        wait_fences_.push_back(std::move(dx_fence));
    }

    void D3D12Queue::signal_api(SemaphoreHandle semaphore)
    {
        ComPtr<ID3D12Fence> dx_fence = context_->get_semaphore(semaphore);
        ORION_ASSERT(dx_fence != nullptr);
        signal_fences_.push_back(std::move(dx_fence));
    }

    void D3D12Queue::submit_api(std::span<const class CommandList* const> command_lists, FenceHandle fence)
    {
        // Add fence to signal_fences if valid
        if (auto dx_fence = context_->get_fence(fence)) {
            signal_fences_.push_back(std::move(dx_fence));
        }

        // Add waits to queue
        for (const auto& dx_fence : wait_fences_) {
            hr_assert(queue_->Wait(dx_fence.Get(), 1));
        }

        // Get ID3D12CommandLists
        command_lists_.resize(command_lists.size());
        std::ranges::transform(command_lists, command_lists_.begin(), [](const CommandList* command_list) {
            const auto* dx_command_list = dynamic_cast<const D3D12CommandList*>(command_list);
            ORION_ASSERT(dx_command_list != nullptr);
            return dx_command_list->dx_command_list().Get();
        });

        // Execute the command lists
        queue_->ExecuteCommandLists(static_cast<UINT>(command_lists_.size()), command_lists_.data());

        // Add signals to queue
        for (const auto& dx_fence : signal_fences_) {
            hr_assert(queue_->Signal(dx_fence.Get(), 1));
        }

        // Reset wait fences
        for (const auto& dx_fence : wait_fences_) {
            hr_assert(dx_fence->Signal(0));
        }
    }
} // namespace orion
