#pragma once

#include "orion-core/config.h"
#include "orion-renderapi/types.h"

#include <cstddef>
#include <memory>
#include <orion-core/exception.h>
#include <spdlog/logger.h>
#include <string_view>
#include <vector>

namespace orion
{
    class DxcInitError : public OrionException
    {
    public:
        explicit DxcInitError(long hresult);

        [[nodiscard]] const char* type() const noexcept override { return "DxcInitError"; }
        [[nodiscard]] int return_code() const noexcept override { return hresult_; }

    private:
        long hresult_;
    };

    enum class ShaderCompileError : int {
        InternalError,
        CompilationFail
    };

    class DxcCompileError : public OrionException
    {
    public:
        explicit DxcCompileError(ShaderCompileError compile_error, const char* msg = "Shader compilation error");

        [[nodiscard]] const char* type() const noexcept override { return "DxcCompileError"; }
        [[nodiscard]] int return_code() const noexcept override { return static_cast<int>(compile_error_); }
        [[nodiscard]] const char* what() const override { return msg_; }

    private:
        ShaderCompileError compile_error_;
        const char* msg_;
    };

    namespace detail
    {
        struct DxcInstance;
        DxcInstance* dxc_create_instance();
        void dxc_destroy_instance(DxcInstance* instance);
        using DxcInstancePtr = std::unique_ptr<DxcInstance, decltype(&dxc_destroy_instance)>;
    } // namespace detail

    struct ShaderCompileDesc {
        std::string source_string;
        std::string source_file;
        const char* entry_point = "main";
        ShaderStage shader_type;
        ShaderObjectType object_type;
        bool enable_debug = orion::debug_build;
    };

    struct ShaderCompileResult {
        std::vector<std::byte> binary;
    };

    class ShaderCompiler
    {
    public:
        ShaderCompiler();

        ShaderCompileResult compile(const ShaderCompileDesc& desc) const;

        static spdlog::logger* logger();

    private:
        detail::DxcInstancePtr dxc_instance_;
    };
} // namespace orion
