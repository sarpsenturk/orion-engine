#pragma once

#include "orion-renderapi/descriptor.h"
#include "orion-renderapi/shader.h"

#include "orion-utils/expected.h"

#include <cstddef>
#include <span>
#include <string>
#include <vector>

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
        std::uint32_t binding;
        std::string name;
        DescriptorType type;
        std::uint32_t count;

        // Bindings are equal if they are the same except for their names which are only for semantics
        bool operator==(const ShaderReflectionDescriptorBinding& other) const noexcept;
    };

    struct ShaderReflectionDescriptorSet {
        std::uint32_t set;
        std::vector<ShaderReflectionDescriptorBinding> bindings;

        bool operator==(const ShaderReflectionDescriptorSet&) const = default;
    };

    struct ShaderReflectionPushConstant {
        std::uint32_t size;
    };

    struct ShaderReflection {
        std::string entry_point;
        ShaderStageFlags stage;
        std::vector<ShaderReflectionInputVariable> input_variables;
        std::vector<ShaderReflectionDescriptorSet> descriptors;
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
