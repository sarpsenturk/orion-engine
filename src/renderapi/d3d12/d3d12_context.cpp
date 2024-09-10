#include "d3d12_context.h"

#include <fmt/base.h>

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
} // namespace orion
