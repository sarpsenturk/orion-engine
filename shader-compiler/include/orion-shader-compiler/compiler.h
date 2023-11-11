#pragma once

#include "orion-utils/expected.h"

#include <string>
#include <string_view>
#include <vector>

namespace orion
{
    enum class ShaderIL {
        SpirV,
        DXIL,
    };

    struct ShaderCompileDesc {
        std::string filename;
        std::string entry_point;
        std::string target_profile;
        ShaderIL shader_il;
        bool debug;
    };

    struct ShaderCompileSuccess {
        std::vector<char> object;
    };

    enum class ShaderCompileError {};

    struct ShaderCompileFail {
        ShaderCompileError error;
        std::string message;
    };

    using ShaderCompileResult = expected<ShaderCompileSuccess, ShaderCompileFail>;

    [[nodiscard]] bool is_vertex_shader(std::string_view profile);
    [[nodiscard]] bool is_pixel_shader(std::string_view profile);
    [[nodiscard]] bool is_geometry_shader(std::string_view profile);
    [[nodiscard]] bool is_hull_shader(std::string_view profile);
    [[nodiscard]] bool is_compute_shader(std::string_view profile);

    namespace shader_compiler
    {
        [[nodiscard]] ShaderCompileResult compile_string(const std::string& source, const ShaderCompileDesc& desc);
        [[nodiscard]] ShaderCompileResult compile_file(const std::string& filename, const ShaderCompileDesc& desc);
    } // namespace shader_compiler
} // namespace orion
