#pragma once

#include "orion/renderapi/handle.h"

#include "orion_dx12.h"

#include <array>

namespace orion
{
    class D3D12HandleTable
    {
    public:
        GraphicsPipelineHandle insert(ComPtr<ID3D12PipelineState> graphics_pipeline);

        ComPtr<ID3D12PipelineState> get(GraphicsPipelineHandle handle);

        bool remove(GraphicsPipelineHandle handle);

    private:
        template<typename T>
        struct Entry {
            ComPtr<T> ptr = nullptr;
            std::uint32_t gen = 0;

            [[nodiscard]] bool is_empty() const noexcept { return ptr == nullptr; }
        };

        std::array<Entry<ID3D12PipelineState>, 0x0ffff> pipelines_;
    };
} // namespace orion
