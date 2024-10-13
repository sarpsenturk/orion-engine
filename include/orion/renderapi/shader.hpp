#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace orion
{
    enum class ShaderType {
        Vertex,
        Pixel,
    };

    struct ShaderCompileOptions {
        std::string source;
        ShaderType type;
    };

    class ShaderCompiler
    {
    public:
        ShaderCompiler() = default;
        virtual ~ShaderCompiler() = default;

        std::vector<std::byte> compile(const ShaderCompileOptions& options);

    protected:
        ShaderCompiler(const ShaderCompiler&) = default;
        ShaderCompiler& operator=(const ShaderCompiler&) = default;
        ShaderCompiler(ShaderCompiler&&) = default;
        ShaderCompiler& operator=(ShaderCompiler&&) = default;

    private:
        virtual std::vector<std::byte> compile_api(const ShaderCompileOptions& options) = 0;
    };
} // namespace orion
