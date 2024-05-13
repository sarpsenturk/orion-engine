#pragma once

#include "orion-renderapi/defs.h"

#include "orion-utils/expected.h"

#include <cstddef>
#include <span>
#include <string>

namespace orion
{
    namespace shader_builtins
    {
        inline constexpr auto unknown = "BuiltInUnknown";
        inline constexpr auto vertex_index = "BuiltInVertexIndex";
    } // namespace shader_builtins

    struct ShaderReflectionInputVariable {
        std::string name;
        Format format;
        bool builtin;
    };

    struct ShaderReflectionDescriptorBinding {
        std::string name;
        DescriptorType type;
        std::uint32_t count;
    };

    struct ShaderReflectionDescriptor {
        std::vector<ShaderReflectionDescriptorBinding> bindings;
    };

    struct ShaderReflectionPushConstant {
        std::uint32_t size;
    };

    struct ShaderReflection {
        std::string entry_point;
        ShaderStageFlags stage;
        std::vector<ShaderReflectionInputVariable> input_variables;
        std::vector<ShaderReflectionDescriptor> descriptors;
        std::vector<ShaderReflectionPushConstant> push_constants;
    };

    enum class ShaderReflectionError {
        Success = 0,
        InternalError,
    };

    using ShaderReflectionResult = expected<ShaderReflection, ShaderReflectionError>;

    class ShaderReflector
    {
    public:
        ShaderReflector() = default;
        virtual ~ShaderReflector() = default;

        [[nodiscard]] ShaderReflectionResult reflect(std::span<const std::byte> shader_code);

    protected:
        ShaderReflector(const ShaderReflector&) = default;
        ShaderReflector(ShaderReflector&&) noexcept = default;
        ShaderReflector& operator=(const ShaderReflector&) = default;
        ShaderReflector& operator=(ShaderReflector&&) noexcept = default;

    private:
        [[nodiscard]] ShaderReflectionResult virtual reflect_api(std::span<const std::byte> shader_code) = 0;
    };
} // namespace orion
