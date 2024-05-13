#include "orion-renderapi/shader_reflection.h"

#include "orion-renderapi/render_device.h"

#include <algorithm>

namespace orion
{
    ShaderReflectionResult ShaderReflector::reflect(std::span<const std::byte> shader_code)
    {
        return reflect_api(shader_code);
    }

    bool operator<(const ShaderReflectionDescriptorBinding& lhs, const ShaderReflectionDescriptorBinding& rhs)
    {
        return lhs.type == rhs.type && lhs.count == rhs.count;
    }

    bool operator<(const ShaderReflectionDescriptorSet& lhs, const ShaderReflectionDescriptorSet& rhs)
    {
        return lhs.set < rhs.set;
    }
} // namespace orion
