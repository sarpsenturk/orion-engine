#pragma once

#include "orion-renderapi/types.h"

#include "orion-utils/expected.h"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace orion
{
    namespace detail
    {
        struct DxcInstance;
        DxcInstance* dxc_create_instance();
        void dxc_destroy_instance(DxcInstance* instance);
        using DxcInstancePtr = std::unique_ptr<DxcInstance, decltype(&dxc_destroy_instance)>;
    } // namespace detail

    struct ShaderCompileDesc {
        const char* entry_point = "main";
        ShaderStageFlags shader_stage;
        ShaderObjectType object_type;
    };

    enum class ShaderCompileError {
        InternalError,
        CompilationFail,
        InvalidSource,
        FailedLoadFile
    };

    constexpr auto format_as(ShaderCompileError compile_error)
    {
        switch (compile_error) {
            case ShaderCompileError::InternalError:
                return "An internal compiler error has occurred in DXC";
            case ShaderCompileError::CompilationFail:
                return "Shader compilation failed";
            case ShaderCompileError::InvalidSource:
                return "Shader source could not be loaded into blob";
            case ShaderCompileError::FailedLoadFile:
                return "Shader file could not opened/loaded";
        }
        return "";
    }

    struct ShaderCompileSuccess {
        std::vector<std::byte> binary;
    };

    using ShaderCompileResult = expected<ShaderCompileSuccess, ShaderCompileError>;

    class ShaderCompiler
    {
    public:
        ShaderCompiler();

        [[nodiscard]] ShaderCompileResult compile_from_source(const std::string& source, const ShaderCompileDesc& desc) const;
        [[nodiscard]] ShaderCompileResult compile_from_file(const std::string& source_file, const ShaderCompileDesc& desc) const;

    private:
        detail::DxcInstancePtr dxc_instance_;
    };

    using shader_compiler_compile_fn = ShaderCompileResult (ShaderCompiler::*)(const std::string&, const ShaderCompileDesc&) const;
} // namespace orion
