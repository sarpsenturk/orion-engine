#pragma once

#include "orion/renderapi/format.h"
#include "orion/renderapi/pipeline.h"

#include "orion_dx12.h"

namespace orion
{
    DXGI_FORMAT to_dxgi_format(Format format);
    D3D12_BLEND to_d3d12_blend(Blend blend);
    D3D12_BLEND_OP to_d3d12_blend_op(BlendOp blend_op);
    UINT8 to_d3d12_write_mask(ColorWriteFlags color_write_flag);
    D3D12_FILL_MODE to_d3d12_fill_mode(FillMode fill_mode);
    D3D12_CULL_MODE to_d3d12_cull_mode(CullMode cull_mode);
    D3D12_PRIMITIVE_TOPOLOGY_TYPE to_d3d12_primitive_topology(PrimitiveTopology topology);
} // namespace orion
