#pragma once

#include "d3d12_device.h"
#include "orion/renderapi/render_command.h"

#include "orion_dx12.h"

namespace orion
{
    class D3D12CommandList final : public CommandList
    {
    public:
        explicit D3D12CommandList(ComPtr<ID3D12GraphicsCommandList> command_list);

        [[nodiscard]] ComPtr<ID3D12GraphicsCommandList> dx_command_list() const { return command_list_; }

    private:
        void begin_api() override;
        void end_api() override;

        ComPtr<ID3D12GraphicsCommandList> command_list_;
    };

    class D3D12CommandAllocator final : public CommandAllocator
    {
    public:
        explicit D3D12CommandAllocator(ComPtr<ID3D12CommandAllocator> command_allocator);

        [[nodiscard]] ComPtr<ID3D12CommandAllocator> d3d12_command_allocator() const { return command_allocator_; }

    private:
        void reset_api() override;

        ComPtr<ID3D12CommandAllocator> command_allocator_;
    };
} // namespace orion
