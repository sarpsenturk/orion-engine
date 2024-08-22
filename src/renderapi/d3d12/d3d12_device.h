#pragma once

#include "orion/renderapi/render_device.h"

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

        ComPtr<ID3D12Device> device_;
        ComPtr<IDXGIFactory2> factory_;
    };
} // namespace orion
