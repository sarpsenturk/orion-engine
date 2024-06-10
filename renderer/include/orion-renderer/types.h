#pragma once

#include "orion-math/matrix/matrix4.h"

#include <cstdint>

namespace orion
{
    struct SceneCBuffer {
        Matrix4_f view_projection;
    };

    struct RenderObjGPUData {
        Matrix4_f transform;
    };

    using mesh_id_t = std::uint16_t;
    using material_id_t = std::uint16_t;
    using texture_id_t = std::uint16_t;
} // namespace orion
