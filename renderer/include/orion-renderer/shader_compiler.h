#pragma once

#include "orion-core/config.h"

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
        bool enable_debug = orion::debug_build;
    };

    enum class ShaderCompileError {
        InternalError,
        CompilationFail,
        InvalidSource,
        FailedLoadFile
    };

    struct ShaderCompileSuccess {
        std::vector<std::byte> binary;
    };

    using ShaderCompileResult = expected<ShaderCompileSuccess, ShaderCompileError>;

    class ShaderCompiler
    {
    public:
        ShaderCompiler();

        [[nodiscard]] ShaderCompileResult compile_from_source(std::string_view source, const ShaderCompileDesc& desc) const;
        [[nodiscard]] ShaderCompileResult compile_from_file(const std::string& source_file, const ShaderCompileDesc& desc) const;

    private:
        detail::DxcInstancePtr dxc_instance_;
    };
} // namespace orion
