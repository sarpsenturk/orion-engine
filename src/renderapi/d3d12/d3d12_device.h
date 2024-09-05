#pragma once

#include "orion/renderapi/render_device.h"

#include "d3d12_handle.h"
#include "orion_dx12.h"

namespace orion
{
    class D3D12Device final : public RenderDevice
    {
    public:
        D3D12Device(ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory2> factory);

    private:
        std::unique_ptr<CommandQueue> create_command_queue_api() override;
        std::unique_ptr<Swapchain> create_swapchain_api(const SwapchainDesc& desc) override;
        std::unique_ptr<ShaderCompiler> create_shader_compiler_api() override;
        std::unique_ptr<CommandAllocator> create_command_allocator_api(const CommandAllocatorDesc& desc) override;
        std::unique_ptr<CommandList> create_command_list_api(const CommandListDesc& desc) override;

        PipelineLayoutHandle create_pipeline_layout_api(const PipelineLayoutDesc& desc) override;
        PipelineHandle create_graphics_pipeline_api(const GraphicsPipelineDesc& desc) override;

        void destroy_api(PipelineLayoutHandle pipeline_layout) override;
        void destroy_api(PipelineHandle pipeline) override;

        ComPtr<ID3D12Device> device_;
        ComPtr<IDXGIFactory2> factory_;

        D3D12RootSignatureTable root_signatures_;
        D3D12PipelineTable pipelines_;
    };
} // namespace orion
