#pragma once

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

        std::size_t hash() const;
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
} // namespace orion
