#include "d3d12_context.h"

#include <fmt/base.h>

#include <utility>

namespace orion
{
    PipelineLayoutHandle D3D12Context::insert_pipeline_layout(ComPtr<ID3D12RootSignature> root_signature)
    {
        return insert<PipelineLayoutHandle>(root_signatures_, std::move(root_signature));
    }

    PipelineHandle D3D12Context::insert_pipeline(ComPtr<ID3D12PipelineState> pipeline)
    {
        return insert<PipelineHandle>(pipelines_, std::move(pipeline));
    }

    BufferHandle D3D12Context::insert_buffer(ComPtr<D3D12MA::Allocation> allocation)
    {
        return insert<BufferHandle>(buffers_, std::move(allocation));
    }

    ImageHandle D3D12Context::insert_image(ComPtr<ID3D12Resource> image)
    {
        return insert<ImageHandle>(images_, std::move(image));
    }

    RenderTargetHandle D3D12Context::insert_render_target(ComPtr<ID3D12DescriptorHeap> descriptor_heap, D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle)
    {
        return insert<RenderTargetHandle>(rtvs_, D3D12RenderTarget{.descriptor_heap = std::move(descriptor_heap), .descriptor_handle = descriptor_handle});
    }

    SemaphoreHandle D3D12Context::insert_semaphore(ComPtr<ID3D12Fence> fence)
    {
        return insert<SemaphoreHandle>(fences_, std::move(fence));
    }

    FenceHandle D3D12Context::insert_fence(ComPtr<ID3D12Fence> fence)
    {
        return insert<FenceHandle>(fences_, std::move(fence));
    }

    ComPtr<ID3D12RootSignature> D3D12Context::get_root_signature(PipelineLayoutHandle pipeline_layout) const
    {
        return lookup(root_signatures_, pipeline_layout);
    }

    ComPtr<ID3D12PipelineState> D3D12Context::get_pipeline(PipelineHandle pipeline) const
    {
        return lookup(pipelines_, pipeline);
    }

    ComPtr<D3D12MA::Allocation> D3D12Context::get_buffer(BufferHandle buffer) const
    {
        return lookup(buffers_, buffer);
    }

    ComPtr<ID3D12Resource> D3D12Context::get_image(ImageHandle image) const
    {
        return lookup(images_, image);
    }

    D3D12RenderTarget D3D12Context::get_render_target(RenderTargetHandle render_target)
    {
        return lookup(rtvs_, render_target);
    }

    ComPtr<ID3D12Fence> D3D12Context::get_semaphore(SemaphoreHandle fence) const
    {
        return lookup(fences_, fence);
    }

    ComPtr<ID3D12Fence> D3D12Context::get_fence(FenceHandle fence) const
    {
        return lookup(fences_, fence);
    }

    bool D3D12Context::remove_root_signature(PipelineLayoutHandle pipeline_layout)
    {
        return remove(root_signatures_, pipeline_layout);
    }

    bool D3D12Context::remove_pipeline(PipelineHandle pipeline)
    {
        return remove(pipelines_, pipeline);
    }

    bool D3D12Context::remove_buffer(BufferHandle buffer)
    {
        return remove(buffers_, buffer);
    }

    bool D3D12Context::remove_image(ImageHandle image)
    {
        return remove(images_, image);
    }

    bool D3D12Context::remove_render_target(RenderTargetHandle render_target)
    {
        return remove(rtvs_, render_target);
    }

    bool D3D12Context::remove_semaphore(SemaphoreHandle semaphore)
    {
        return remove(fences_, semaphore);
    }

    bool D3D12Context::remove_fence(FenceHandle fence)
    {
        return remove(fences_, fence);
    }
} // namespace orion
