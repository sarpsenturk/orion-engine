#pragma once

#include "orion-renderapi/types.h"

#include "orion-utils/expected.h"

#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>

namespace orion
{
    enum class ShaderCompileError {};

    struct ShaderCompileFail {
        ShaderCompileError error;
        const char* message;
    };

    class ShaderObject
    {
    public:
        ShaderObject(const ShaderObject&) = delete;
        ShaderObject(ShaderObject&&) noexcept;
        ShaderObject& operator=(const ShaderObject&) = delete;
        ShaderObject& operator=(ShaderObject&&) noexcept;
        ~ShaderObject();

        std::vector<std::byte> get_binary() const;

    private:
        struct ShaderObjectImpl;
        std::unique_ptr<ShaderObjectImpl> impl_;

        // Only ShaderCompiler can create shader objects
        explicit ShaderObject(std::unique_ptr<ShaderObjectImpl> data);
        friend class ShaderCompiler;
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

        expected<ShaderObject, ShaderCompileFail> compile(const ShaderCompileDesc& desc) const;

    private:
        struct ShaderCompilerImpl;
        std::unique_ptr<ShaderCompilerImpl> impl_;
    };
} // namespace orion
