#pragma once

#include "orion/renderapi/render_device.h"

#include "orion_dx12.h"

#include "d3d12_context.h"

#include "D3D12MemAlloc.h"

namespace orion
{
    class D3D12Device final : public RenderDevice
    {
    public:
        D3D12Device(ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory2> factory, ComPtr<IDXGIAdapter1> adapter);

    private:
        std::unique_ptr<CommandQueue> create_command_queue_api() override;
        std::unique_ptr<Swapchain> create_swapchain_api(const SwapchainDesc& desc) override;
        std::unique_ptr<ShaderCompiler> create_shader_compiler_api() override;
        std::unique_ptr<CommandAllocator> create_command_allocator_api(const CommandAllocatorDesc& desc) override;
        std::unique_ptr<CommandList> create_command_list_api(const CommandListDesc& desc) override;
        BufferHandle create_buffer_api(const BufferDesc& desc) override;

        PipelineLayoutHandle create_pipeline_layout_api(const PipelineLayoutDesc& desc) override;
        PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) override;
        SemaphoreHandle create_semaphore_api(const SemaphoreDesc& desc) override;
        FenceHandle create_fence_api(const FenceDesc& desc) override;

        void destroy_api(PipelineLayoutHandle pipeline_layout) override;
        void destroy_api(PipelineHandle pipeline) override;
        void destroy_api(BufferHandle buffer) override;
        void destroy_api(SemaphoreHandle semaphore) override;
        void destroy_api(FenceHandle fence) override;

        void* map_api(BufferHandle buffer) override;
        void unmap_api(BufferHandle buffer) override;

        void wait_for_fence_api(FenceHandle fence) override;

        ComPtr<ID3D12Device> device_;
        ComPtr<IDXGIFactory2> factory_;
        ComPtr<IDXGIAdapter1> adapter_;
        ComPtr<D3D12MA::Allocator> allocator_;

        D3D12Context context_;
    };
} // namespace orion
