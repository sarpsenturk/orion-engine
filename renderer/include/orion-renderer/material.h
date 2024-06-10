#pragma once

#include "orion-renderer/colors.h"
#include "orion-renderer/types.h"

#include "orion-renderapi/device_resource.h"

namespace orion
{
    struct MaterialData {
        Color color = colors::white;
    };

    class Material
    {
    public:
        Material(UniqueGPUBuffer constant_buffer, UniqueDescriptor descriptor, MaterialData data);

        DescriptorHandle descriptor() const noexcept { return descriptor_.get(); }
        texture_id_t texture_id() const noexcept { return texture_id_; }
        const Color& color() const noexcept { return material_data_.color; }

    private:
        UniqueGPUBuffer constant_buffer_;
        UniqueDescriptor descriptor_;
        MaterialData material_data_;
        texture_id_t texture_id_ = 0;
    };
} // namespace orion
