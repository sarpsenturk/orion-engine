#pragma once

#include "orion-renderer/colors.h"

#include "orion-renderapi/device_resource.h"

namespace orion
{
    struct MaterialData {
        Color color = colors::white;
    };

    // Forward declare
    class Effect;
    class RenderContext;

    class Material
    {
    public:
        Material(const Effect* effect, UniqueGPUBuffer constant_buffer, UniqueDescriptor descriptor, MaterialData data);

        const Effect* effect() const noexcept { return effect_; }
        DescriptorHandle descriptor() const noexcept { return descriptor_.get(); }
        const Color& color() const noexcept { return material_data_.color; }

    private:
        const Effect* effect_;
        UniqueGPUBuffer constant_buffer_;
        UniqueDescriptor descriptor_;
        MaterialData material_data_;
    };

    class MaterialBuilder
    {
    public:
        static constexpr auto max_materials = 64;

        MaterialBuilder(RenderContext* context, DescriptorLayoutHandle material_layout);

        Material create(const Effect* effect, const MaterialData& data);

    private:
        RenderContext* context_;
        DescriptorLayoutHandle material_layout_;
        UniqueDescriptorPool descriptor_pool_;
    };
} // namespace orion
