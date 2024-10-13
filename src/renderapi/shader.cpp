#include "orion/renderapi/shader.hpp"

namespace orion
{
    std::vector<std::byte> ShaderCompiler::compile(const ShaderCompileOptions& options)
    {
        return compile_api(options);
    }
} // namespace orion
