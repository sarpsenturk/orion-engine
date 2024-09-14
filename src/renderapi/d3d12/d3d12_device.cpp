#include "d3d12_device.h"

#include "d3d12_command.h"
#include "d3d12_conversion.h"
#include "d3d12_queue.h"
#include "d3d12_shader.h"
#include "d3d12_swapchain.h"

#include "win32/win32_window.h"

#include "orion/assertion.h"

#include <spdlog/spdlog.h>

#include <algorithm>

namespace orion
{
    D3D12Device::D3D12Device(ComPtr<ID3D12Device> device, ComPtr<IDXGIFactory2> factory, ComPtr<IDXGIAdapter1> adapter)
        : device_(std::move(device))
        , factory_(std::move(factory))
        , adapter_(std::move(adapter))
    {
        // Create D3D12MA allocator
        {
            const auto desc = D3D12MA::ALLOCATOR_DESC{
                .Flags = D3D12MA::ALLOCATOR_FLAG_MSAA_TEXTURES_ALWAYS_COMMITTED |
                         D3D12MA::ALLOCATOR_FLAG_DEFAULT_POOLS_NOT_ZEROED,
                .pDevice = device_.Get(),
                .pAdapter = adapter_.Get(),
            };
            hr_assert(D3D12MA::CreateAllocator(&desc, &allocator_));
            SPDLOG_TRACE("Created D3D12MA::Allocator interface at {}", fmt::ptr(allocator_.Get()));
        }
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

    std::unique_ptr<Swapchain> D3D12Device::create_swapchain_api(const SwapchainDesc& desc)
    {
        const auto dxgi_desc = DXGI_SWAP_CHAIN_DESC1{
            .Width = desc.width,
            .Height = desc.height,
            .Format = to_dxgi_format(desc.image_format),
            .Stereo = FALSE,
            .SampleDesc = {.Count = 1, .Quality = 0},
            .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
            .BufferCount = desc.image_count,
            .Scaling = DXGI_SCALING_STRETCH,
            .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
            .AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
            .Flags = 0,
        };

        ID3D12CommandQueue* queue = static_cast<const D3D12Queue*>(desc.queue)->get();
        HWND hwnd = desc.window->platform_window()->hwnd;

        ComPtr<IDXGISwapChain1> swapchain;
        hr_assert(factory_->CreateSwapChainForHwnd(queue, hwnd, &dxgi_desc, nullptr, nullptr, &swapchain));
        SPDLOG_TRACE("Created IDXGISwapChain1 interface at {}", fmt::ptr(swapchain.Get()));

        return std::make_unique<D3D12Swapchain>(device_.Get(), std::move(swapchain), &context_);
    }

    std::unique_ptr<ShaderCompiler> D3D12Device::create_shader_compiler_api()
    {
        return std::make_unique<D3D12ShaderCompiler>();
    }

    std::unique_ptr<CommandAllocator> D3D12Device::create_command_allocator_api([[maybe_unused]] const CommandAllocatorDesc& desc)
    {
        ComPtr<ID3D12CommandAllocator> command_allocator;
        hr_assert(device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)));
        SPDLOG_TRACE("Created ID3D12CommandAllocator interface at {}", fmt::ptr(command_allocator.Get()));
        return std::make_unique<D3D12CommandAllocator>(std::move(command_allocator));
    }

    std::unique_ptr<CommandList> D3D12Device::create_command_list_api(const CommandListDesc& desc)
    {
        const auto command_allocator = dynamic_cast<const D3D12CommandAllocator*>(desc.command_allocator)->d3d12_command_allocator();
        ORION_ASSERT(command_allocator != nullptr);
        ComPtr<ID3D12CommandList> command_list;
        hr_assert(device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator.Get(), nullptr, IID_PPV_ARGS(&command_list)));
        SPDLOG_TRACE("Created ID3D12CommandList interface at {}", fmt::ptr(command_list.Get()));
        return std::make_unique<D3D12CommandList>(std::move(command_list));
    }

    BufferHandle D3D12Device::create_buffer_api(const BufferDesc& desc)
    {
        ComPtr<D3D12MA::Allocation> allocation;
        {
            const auto resource_desc = D3D12_RESOURCE_DESC{
                .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
                .Alignment = 0,
                .Width = desc.size,
                .Height = 1,
                .DepthOrArraySize = 1,
                .MipLevels = 1,
                .Format = DXGI_FORMAT_UNKNOWN,
                .SampleDesc = {.Count = 1, .Quality = 0},
                .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
                .Flags = D3D12_RESOURCE_FLAG_NONE,
            };
            const auto alloc_desc = D3D12MA::ALLOCATION_DESC{
                .HeapType = desc.cpu_visible ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT,
            };
            hr_assert(
                allocator_->CreateResource(
                    &alloc_desc,
                    &resource_desc,
                    desc.cpu_visible ? D3D12_RESOURCE_STATE_GENERIC_READ : to_d3d12_buffer_state(desc.usage),
                    nullptr, &allocation,
                    IID_NULL,
                    nullptr));
            SPDLOG_TRACE("Created ID3D12Resource (buffer) {} with D3D12MA::Allocation {}", fmt::ptr(allocation->GetResource()), fmt::ptr(allocation.Get()));
        }
        return context_.insert_buffer(std::move(allocation));
    }

    PipelineLayoutHandle D3D12Device::create_pipeline_layout_api([[maybe_unused]] const PipelineLayoutDesc& desc)
    {
        const auto root_signature_desc = D3D12_ROOT_SIGNATURE_DESC{
            .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
        };

        ComPtr<ID3D10Blob> root_signature_blob;
        hr_assert(D3D12SerializeRootSignature(&root_signature_desc, D3D_ROOT_SIGNATURE_VERSION_1, &root_signature_blob, nullptr));

        ComPtr<ID3D12RootSignature> root_signature;
        hr_assert(device_->CreateRootSignature(0, root_signature_blob->GetBufferPointer(), root_signature_blob->GetBufferSize(), IID_PPV_ARGS(&root_signature)));
        SPDLOG_TRACE("Created ID3D12RootSignature interface at {}", fmt::ptr(root_signature.Get()));

        return context_.insert_pipeline_layout(std::move(root_signature));
    }

    PipelineHandle D3D12Device::create_graphics_pipeline_api(const GraphicsPipelineDesc& desc)
    {
        ComPtr<ID3D12PipelineState> pipeline;
        {
            D3D12_RENDER_TARGET_BLEND_DESC render_target_blend[8];
            std::ranges::transform(desc.blend.render_targets, render_target_blend, [](const RenderTargetBlendDesc& render_target) {
                return D3D12_RENDER_TARGET_BLEND_DESC{
                    .BlendEnable = render_target.blend_enable,
                    .LogicOpEnable = FALSE,
                    .SrcBlend = to_d3d12_blend(render_target.src_blend),
                    .DestBlend = to_d3d12_blend(render_target.dst_blend),
                    .BlendOp = to_d3d12_blend_op(render_target.blend_op),
                    .SrcBlendAlpha = to_d3d12_blend(render_target.src_blend_alpha),
                    .DestBlendAlpha = to_d3d12_blend(render_target.dst_blend_alpha),
                    .BlendOpAlpha = to_d3d12_blend_op(render_target.blend_op_alpha),
                    .LogicOp = D3D12_LOGIC_OP_NOOP,
                    .RenderTargetWriteMask = to_d3d12_write_mask(render_target.color_write_mask),
                };
            });

            const auto blend_state = D3D12_BLEND_DESC{
                .AlphaToCoverageEnable = FALSE,
                .IndependentBlendEnable = TRUE,
                .RenderTarget = *render_target_blend,
            };

            const auto rasterizer_state = D3D12_RASTERIZER_DESC{
                .FillMode = to_d3d12_fill_mode(desc.rasterizer.fill_mode),
                .CullMode = to_d3d12_cull_mode(desc.rasterizer.cull_mode),
                .FrontCounterClockwise = desc.rasterizer.front_face == FrontFace::CounterClockWise ? TRUE : FALSE,
                .DepthBias = 1,
                .DepthBiasClamp = 1.f,
                .SlopeScaledDepthBias = 1.f,
                .DepthClipEnable = FALSE,
                .MultisampleEnable = FALSE,
                .AntialiasedLineEnable = FALSE,
                .ForcedSampleCount = 0,
                .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,
            };

            const auto depth_stencil_state = D3D12_DEPTH_STENCIL_DESC{
                .DepthEnable = FALSE,
                .DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL,
                .DepthFunc = D3D12_COMPARISON_FUNC_LESS,
                .StencilEnable = FALSE,
                .StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK,
                .StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK,
                .FrontFace = {
                    .StencilFailOp = D3D12_STENCIL_OP_KEEP,
                    .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
                    .StencilPassOp = D3D12_STENCIL_OP_KEEP,
                    .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS,
                },
                .BackFace = {
                    .StencilFailOp = D3D12_STENCIL_OP_KEEP,
                    .StencilDepthFailOp = D3D12_STENCIL_OP_KEEP,
                    .StencilPassOp = D3D12_STENCIL_OP_KEEP,
                    .StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS,
                },
            };

            std::vector<D3D12_INPUT_ELEMENT_DESC> input_elements(desc.vertex_attributes.size());
            std::ranges::transform(desc.vertex_attributes, input_elements.begin(), [index = 0u](const VertexAttribute& attribute) mutable {
                return D3D12_INPUT_ELEMENT_DESC{
                    .SemanticName = attribute.name,
                    .SemanticIndex = index++,
                    .Format = to_dxgi_format(attribute.format),
                    .InputSlot = 0,
                    .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
                    .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                    .InstanceDataStepRate = 0,
                };
            });

            const auto input_layout = D3D12_INPUT_LAYOUT_DESC{
                .pInputElementDescs = input_elements.data(),
                .NumElements = static_cast<UINT>(input_elements.size()),
            };

            DXGI_FORMAT rtv_formats[8] = {};
            std::ranges::transform(desc.render_target_formats, rtv_formats, [](Format format) {
                return to_dxgi_format(format);
            });

            const auto multisample = DXGI_SAMPLE_DESC{
                .Count = 1,
                .Quality = 0,
            };

            const auto root_signature = context_.get_root_signature(desc.pipeline_layout);
            ORION_EXPECTS(root_signature != nullptr);

            const auto d3d12_desc = D3D12_GRAPHICS_PIPELINE_STATE_DESC{
                .pRootSignature = root_signature.Get(),
                .VS = {.pShaderBytecode = desc.vertex_shader.data(), .BytecodeLength = desc.vertex_shader.size_bytes()},
                .PS = {.pShaderBytecode = desc.pixel_shader.data(), .BytecodeLength = desc.pixel_shader.size_bytes()},
                .BlendState = blend_state,
                .SampleMask = D3D12_DEFAULT_SAMPLE_MASK,
                .RasterizerState = rasterizer_state,
                .DepthStencilState = depth_stencil_state,
                .InputLayout = input_layout,
                .PrimitiveTopologyType = to_d3d12_primitive_topology(desc.primitive_topology),
                .NumRenderTargets = static_cast<UINT>(desc.render_target_formats.size()),
                .RTVFormats = *rtv_formats,
                .DSVFormat = DXGI_FORMAT_UNKNOWN,
                .SampleDesc = multisample,
                .NodeMask = 0,
            };

            hr_assert(device_->CreateGraphicsPipelineState(&d3d12_desc, IID_PPV_ARGS(&pipeline)));
            SPDLOG_TRACE("Created graphics ID3D12Pipeline interface at {}", fmt::ptr(pipeline.Get()));
        }

        return context_.insert_pipeline(std::move(pipeline));
    }

    void D3D12Device::destroy_api(PipelineLayoutHandle pipeline_layout)
    {
        if (!context_.remove_root_signature(pipeline_layout)) {
            SPDLOG_WARN("Trying to remove pipeline layout with handle {}, which is not a valid handle", fmt::underlying(pipeline_layout));
        }
    }

    void D3D12Device::destroy_api(PipelineHandle pipeline)
    {
        if (!context_.remove_pipeline(pipeline)) {
            SPDLOG_WARN("Trying to remove pipeline with handle {}, which is not a valid handle", fmt::underlying(pipeline));
        }
    }

    void D3D12Device::destroy_api(BufferHandle buffer)
    {
        if (!context_.remove_buffer(buffer)) {
            SPDLOG_WARN("Trying to remove buffer with handle {}, which is not a valid handle", fmt::underlying(buffer));
        }
    }

    void* D3D12Device::map_api(BufferHandle buffer)
    {
        if (auto allocation = context_.get_buffer(buffer)) {
            ID3D12Resource* resource = allocation->GetResource();
            void* ptr;
            hr_assert(resource->Map(0, nullptr, &ptr));
            return ptr;
        } else {
            SPDLOG_ERROR("Failed to map buffer resource with handle {}: couldn't find buffer", fmt::underlying(buffer));
            return nullptr;
        }
    }

    void D3D12Device::unmap_api(BufferHandle buffer)
    {
        if (auto allocation = context_.get_buffer(buffer)) {
            ID3D12Resource* resource = allocation->GetResource();
            resource->Unmap(0, nullptr);
        } else {
            SPDLOG_ERROR("Failed to unmap buffer with handle {}: couldn't find buffer", fmt::underlying(buffer));
        }
    }
} // namespace orion
