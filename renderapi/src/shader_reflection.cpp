#include "orion-renderapi/shader_reflection.h"

namespace orion
{
    ShaderReflectionResult ShaderReflector::reflect(std::span<const std::byte> shader_code)
    {
        return reflect_api(shader_code);
    }

    bool ShaderReflectionDescriptorBinding::operator==(const ShaderReflectionDescriptorBinding& other) const noexcept
    {
        return binding == other.binding &&
               type == other.type &&
               count == other.count;
    }
} // namespace orion
