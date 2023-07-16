#pragma once

#include "handles.h"
#include "types.h"

#include <initializer_list>
#include <span>

namespace orion
{
    struct DescriptorPoolSize {
        DescriptorType type;
        std::uint32_t count;
    };

    struct DescriptorBinding {
        DescriptorType type;
        ShaderStageFlags shader_stages;
        std::uint32_t count;
    };

    class DescriptorSetLayout
    {
    public:
        DescriptorSetLayout(std::initializer_list<DescriptorBinding> bindings);

        [[nodiscard]] auto& bindings() const noexcept { return bindings_; }
        [[nodiscard]] auto hash() const noexcept { return hash_; }

    private:
        static std::size_t hash_bindings(std::span<const DescriptorBinding> bindings);

        std::vector<DescriptorBinding> bindings_;
        std::size_t hash_;
    };

    struct DescriptorPoolDesc {
        std::uint32_t max_sets;
        std::span<const DescriptorPoolSize> pool_sizes;
    };

    struct DescriptorSetDesc {
        DescriptorPoolHandle descriptor_pool;
        const DescriptorSetLayout* layout;
    };

    struct DescriptorBuffer {
        GPUBufferHandle buffer = GPUBufferHandle::invalid_handle();
        std::size_t offset = SIZE_MAX;
        std::size_t range = SIZE_MAX;
    };

    struct DescriptorWrite {
        std::uint32_t binding = UINT32_MAX;
        std::uint32_t array_element = 0;
        DescriptorSetHandle descriptor_set = DescriptorSetHandle::invalid_handle();
        DescriptorType descriptor_type = DescriptorType::Unknown;
        std::span<const DescriptorBuffer> buffers = {};
    };

    struct DescriptorUpdate {
        std::span<const DescriptorWrite> writes;
    };
} // namespace orion
