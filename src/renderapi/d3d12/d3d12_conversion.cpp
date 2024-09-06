#include "d3d12_conversion.h"

#include "orion/assertion.h"

namespace orion
{
    DXGI_FORMAT to_dxgi_format(Format format)
    {
        switch (format) {
            case Format::Undefined:
                return DXGI_FORMAT_UNKNOWN;
            case Format::B8G8R8A8_Unorm:
                return DXGI_FORMAT_B8G8R8A8_UNORM;
            case Format::R32G32B32_Float:
                return DXGI_FORMAT_R32G32B32_FLOAT;
        }
        unreachable();
    }

    D3D12_BLEND to_d3d12_blend(Blend blend)
    {
        switch (blend) {
            case Blend::Zero:
                return D3D12_BLEND_ZERO;
            case Blend::One:
                return D3D12_BLEND_ONE;
        }
        unreachable();
    }

    D3D12_BLEND_OP to_d3d12_blend_op(BlendOp blend_op)
    {
        switch (blend_op) {
            case BlendOp::Add:
                return D3D12_BLEND_OP_ADD;
            case BlendOp::Subtract:
                return D3D12_BLEND_OP_SUBTRACT;
        }
        unreachable();
    }

    UINT8 to_d3d12_write_mask(ColorWriteFlags color_write_flag)
    {
        UINT8 mask = 0;
        if ((color_write_flag & ColorWriteFlags::Red) != ColorWriteFlags::None) {
            mask |= D3D12_COLOR_WRITE_ENABLE_RED;
        }
        if ((color_write_flag & ColorWriteFlags::Green) != ColorWriteFlags::None) {
            mask |= D3D12_COLOR_WRITE_ENABLE_GREEN;
        }
        if ((color_write_flag & ColorWriteFlags::Blue) != ColorWriteFlags::None) {
            mask |= D3D12_COLOR_WRITE_ENABLE_BLUE;
        }
        return mask;
    }

    D3D12_FILL_MODE to_d3d12_fill_mode(FillMode fill_mode)
    {
        switch (fill_mode) {
            case FillMode::Solid:
                return D3D12_FILL_MODE_SOLID;
            case FillMode::Wireframe:
                return D3D12_FILL_MODE_WIREFRAME;
        }
        unreachable();
    }

    D3D12_CULL_MODE to_d3d12_cull_mode(CullMode cull_mode)
    {
        switch (cull_mode) {
            case CullMode::None:
                return D3D12_CULL_MODE_NONE;
            case CullMode::Back:
                return D3D12_CULL_MODE_BACK;
            case CullMode::Front:
                return D3D12_CULL_MODE_FRONT;
        }
        unreachable();
    }

    D3D12_PRIMITIVE_TOPOLOGY_TYPE to_d3d12_primitive_topology(PrimitiveTopology topology)
    {
        switch (topology) {
            case PrimitiveTopology::Point:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
            case PrimitiveTopology::Line:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
            case PrimitiveTopology::Triangle:
                return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        }
        unreachable();
    }

    D3D12_RESOURCE_STATES to_d3d12_buffer_state(BufferUsage buffer_usage)
    {
        switch (buffer_usage) {
            case BufferUsage::Vertex:
                return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
            case BufferUsage::Index:
                return D3D12_RESOURCE_STATE_INDEX_BUFFER;
        }
        unreachable();
    }
} // namespace orion
