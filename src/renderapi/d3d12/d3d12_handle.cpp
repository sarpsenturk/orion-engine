#include "d3d12_handle.h"

#include "orion/assertion.h"

namespace orion
{
    namespace
    {
        std::uint16_t find_empty_slot(const auto& table)
        {
            for (std::uint16_t i = 0; i < table.size(); ++i) {
                if (table[i].is_empty()) {
                    return i;
                }
            }
            return UINT16_MAX;
        }

        struct D3D12Handle {
            std::uint16_t slot;
            std::uint16_t gen;
        };

        D3D12Handle to_d3d12_handle(auto handle)
        {
            const auto device_handle = static_cast<render_device_handle_t>(handle);
            return {.slot = device_handle & 0xffff, .gen = device_handle >> 16 & 0xffff};
        }
    } // namespace

    GraphicsPipelineHandle D3D12HandleTable::insert(ComPtr<ID3D12PipelineState> graphics_pipeline)
    {
        const auto slot = find_empty_slot(pipelines_);
        if (slot == UINT16_MAX) {
            return GraphicsPipelineHandle{invalid_device_handle};
        }

        auto& entry = pipelines_[slot];
        entry.ptr = std::move(graphics_pipeline);
        return static_cast<GraphicsPipelineHandle>(entry.gen << 16 | slot);
    }

    ComPtr<ID3D12PipelineState> D3D12HandleTable::get(GraphicsPipelineHandle handle)
    {
        const auto [slot, gen] = to_d3d12_handle(handle);
        ORION_ASSERT(slot < 0xffff);
        if (pipelines_[slot].gen == gen) {
            return pipelines_[slot].ptr;
        } else {
            return nullptr;
        }
    }

    bool D3D12HandleTable::remove(GraphicsPipelineHandle handle)
    {
        const auto [slot, gen] = to_d3d12_handle(handle);
        ORION_ASSERT(slot < 0xffff);
        if (pipelines_[slot].gen == gen) {
            pipelines_[slot].ptr = nullptr;
            pipelines_[slot].gen += 1;
            return true;
        } else {
            return false;
        }
    }
} // namespace orion
