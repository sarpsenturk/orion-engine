#include "orion-renderer/material.h"

#include <utility>

namespace orion
{
    Material::Material(UniqueGPUBuffer constant_buffer, UniqueDescriptor descriptor, MaterialData data)
        : constant_buffer_(std::move(constant_buffer))
        , descriptor_(std::move(descriptor))
        , material_data_(data)
    {
    }
} // namespace orion
