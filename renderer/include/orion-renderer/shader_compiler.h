#pragma once

#include "orion-renderapi/types.h"

#include "orion-utils/expected.h"

#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>

namespace orion
{
    struct ShaderCompileSuccess {
        std::vector<std::byte> binary;
    };

    enum class ShaderCompileError {};

    struct ShaderCompileFail {
        ShaderCompileError error;
        const char* message;
    };

    struct ShaderCompileDesc {
        std::string_view source;
        const char* entry_point;
        ShaderStageFlags stage;
        ShaderObjectType object_type;
    };

    class ShaderCompiler
    {
    public:
        ShaderCompiler();
        ShaderCompiler(const ShaderCompiler&) = delete;
        ShaderCompiler(ShaderCompiler&&) noexcept;
        ShaderCompiler& operator=(const ShaderCompiler&) = delete;
        ShaderCompiler& operator=(ShaderCompiler&&) noexcept;
        ~ShaderCompiler();

        expected<ShaderCompileSuccess, ShaderCompileFail> compile(const ShaderCompileDesc& desc) const;

    private:
        struct ShaderCompilerImpl;
        std::unique_ptr<ShaderCompilerImpl> impl_;
    };
} // namespace orion
