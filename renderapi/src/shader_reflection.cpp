#include "orion-renderapi/shader_reflection.h"

#include "orion-renderapi/render_device.h"

#include <algorithm>

namespace orion
{
    ShaderReflectionResult ShaderReflector::reflect(std::span<const std::byte> shader_code)
    {
        return reflect_api(shader_code);
    }
} // namespace orion
