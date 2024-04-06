#pragma once

#include "orion-renderapi/shader_reflection.h"

#include <cstddef>
#include <span>

namespace orion::vulkan
{
    class VulkanShaderReflector : public ShaderReflector
    {
        [[nodiscard]] ShaderReflectionResult reflect_api(std::span<const std::byte> shader_code) override;
    };
    [[nodiscard]] ShaderReflectionResult get_spirv_reflection(std::span<const std::byte> spirv_code);
} // namespace orion::vulkan
