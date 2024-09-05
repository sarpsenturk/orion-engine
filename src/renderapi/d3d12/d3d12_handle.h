#pragma once

#include "orion/renderapi/handle.h"

#include "orion_dx12.h"

#include <array>

namespace orion
{
    template<typename DxInterface, typename OrionHandle>
    class D3D12HandleTable
    {
    public:
        OrionHandle insert(ComPtr<DxInterface> ptr)
        {
            const auto slot = find_empty_slot();
            if (slot == UINT16_MAX) {
                return OrionHandle::Invalid;
            }
            auto& entry = table_[slot];
            entry.ptr = std::move(ptr);
            return static_cast<OrionHandle>(entry.gen << 16 | slot);
        }

        ComPtr<DxInterface> lookup(OrionHandle handle)
        {
            const auto [slot, gen] = to_d3d12_handle(handle);
            if (table_[slot].gen == gen) {
                return table_[slot].ptr;
            } else {
                return nullptr;
            }
        }

        bool remove(OrionHandle handle)
        {
            const auto [slot, gen] = to_d3d12_handle(handle);
            if (table_[slot].gen == gen) {
                table_[slot].ptr = nullptr;
                table_[slot].gen += 1;
                return true;
            } else {
                return false;
            }
        }

    private:
        struct Entry {
            ComPtr<DxInterface> ptr = nullptr;
            std::uint32_t gen = 0;

            [[nodiscard]] bool is_empty() const noexcept { return ptr == nullptr; }
        };

        std::uint16_t find_empty_slot()
        {
            for (std::uint16_t i = 0; i < table_.size(); ++i) {
                if (table_[i].is_empty()) {
                    return i;
                }
            }
            return UINT16_MAX;
        }

        struct D3D12HandleImpl {
            std::uint16_t slot;
            std::uint16_t gen;
        };

        D3D12HandleImpl to_d3d12_handle(auto handle)
        {
            const auto device_handle = static_cast<render_device_handle_t>(handle);
            return {.slot = device_handle & 0xffff, .gen = device_handle >> 16 & 0xffff};
        }

        std::array<Entry, 0x0ffff> table_;
    };

    using D3D12RootSignatureTable = D3D12HandleTable<ID3D12RootSignature, PipelineLayoutHandle>;
    using D3D12PipelineTable = D3D12HandleTable<ID3D12PipelineState, PipelineHandle>;
} // namespace orion
