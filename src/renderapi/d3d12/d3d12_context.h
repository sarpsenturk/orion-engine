#pragma once

#include "orion/renderapi/handle.h"

#include "orion_dx12.h"

#include "D3D12MemAlloc.h"

#include <array>

namespace orion
{
    struct D3D12RenderTarget {
        ComPtr<ID3D12DescriptorHeap> descriptor_heap = nullptr;
        D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle = {};

        D3D12RenderTarget& operator=(std::nullptr_t)
        {
            descriptor_heap = nullptr;
            descriptor_handle = D3D12_CPU_DESCRIPTOR_HANDLE{};
            return *this;
        }
    };

    class D3D12Context
    {
    public:
        struct D3D12HandleImpl {
            std::uint16_t index;
            std::uint32_t gen;
        };

        static D3D12HandleImpl to_d3d12_handle(auto handle)
        {
            const auto value = static_cast<render_device_handle_t>(handle);
            return {.index = static_cast<std::uint16_t>(value & 0xffff), .gen = static_cast<std::uint16_t>(value >> 16 & 0xffff)};
        }

        PipelineLayoutHandle insert_pipeline_layout(ComPtr<ID3D12RootSignature> root_signature);
        PipelineHandle insert_pipeline(ComPtr<ID3D12PipelineState> pipeline);
        BufferHandle insert_buffer(ComPtr<D3D12MA::Allocation> allocation);
        ImageHandle insert_image(ComPtr<ID3D12Resource> image);
        RenderTargetHandle insert_render_target(ComPtr<ID3D12DescriptorHeap> descriptor_heap, D3D12_CPU_DESCRIPTOR_HANDLE descriptor_handle);

        ComPtr<ID3D12RootSignature> get_root_signature(PipelineLayoutHandle pipeline_layout) const;
        ComPtr<ID3D12PipelineState> get_pipeline(PipelineHandle pipeline) const;
        ComPtr<D3D12MA::Allocation> get_buffer(BufferHandle buffer) const;
        ComPtr<ID3D12Resource> get_image(ImageHandle image) const;
        D3D12RenderTarget get_render_target(RenderTargetHandle render_target);

        bool remove_root_signature(PipelineLayoutHandle pipeline_layout);
        bool remove_pipeline(PipelineHandle pipeline);
        bool remove_buffer(BufferHandle buffer);
        bool remove_image(ImageHandle image);
        bool remove_render_target(RenderTargetHandle render_target);

    private:
        template<typename T>
        struct Entry {
            T value = {};
            std::uint16_t gen = 0;
        };

        template<typename T>
        using ResourceTable = std::array<Entry<T>, 0xffff>;

        template<typename T>
        static bool is_empty_slot(const Entry<T>& entry)
        {
            return entry.value == nullptr;
        }

        template<>
        bool is_empty_slot<D3D12RenderTarget>(const Entry<D3D12RenderTarget>& entry)
        {
            return entry.value.descriptor_heap == nullptr;
        }

        static std::uint16_t find_empty_slot(const auto& table)
        {
            for (std::uint16_t i = 0; i < table.size(); ++i) {
                if (is_empty_slot(table[i])) {
                    return i;
                }
            }
            return UINT16_MAX;
        }

        template<typename T>
        static T insert(auto& table, auto value)
        {
            const auto slot = find_empty_slot(table);
            if (slot == UINT16_MAX) {
                return T::Invalid;
            }
            table[slot].value = std::move(value);
            return static_cast<T>(slot | table[slot].gen << 16);
        }

        template<typename T>
        static T lookup(const ResourceTable<T>& table, auto handle)
        {
            const auto [index, gen] = to_d3d12_handle(handle);
            if (index >= table.size()) {
                return {};
            }
            if (table[index].gen != gen) {
                return {};
            }
            return table[index].value;
        }

        static bool remove(auto& table, auto handle)
        {
            const auto [index, gen] = to_d3d12_handle(handle);
            if (index >= table.size()) {
                return false;
            }
            if (table[index].gen != gen) {
                return false;
            }
            table[index].value = nullptr;
            table[index].gen += 1;
            return true;
        }

        ResourceTable<ComPtr<ID3D12RootSignature>> root_signatures_;
        ResourceTable<ComPtr<ID3D12PipelineState>> pipelines_;
        ResourceTable<ComPtr<D3D12MA::Allocation>> buffers_;
        ResourceTable<ComPtr<ID3D12Resource>> images_;
        ResourceTable<D3D12RenderTarget> rtvs_;
    };
} // namespace orion
