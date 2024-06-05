#include "orion-renderer/material.h"

#include "orion-renderapi/render_device.h"

#include <utility>

namespace orion
{
    Material::Material(const Effect* effect, UniqueGPUBuffer constant_buffer, UniqueDescriptor descriptor, MaterialData data)
        : effect_(effect)
        , constant_buffer_(std::move(constant_buffer))
        , descriptor_(std::move(descriptor))
        , material_data_(data)
    {
    }
} // namespace orion
