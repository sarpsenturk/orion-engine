#include "orion-renderer/shader_compiler.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEA

#include "dxc_include.h"
#include "orion-utils/assertion.h"
#include "orion-utils/static_vector.h"

#include <cstring>                           // std::memcpy
#include <spdlog/sinks/stdout_color_sinks.h> // spdlog::stdout_color_mt
#include <spdlog/spdlog.h>                   // SPDLOG_*

#ifndef FAILED
    #define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif

#include <codecvt>
#include <locale>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace orion
{
    // Internal details about dxc implementation
    namespace detail
    {
        // DirectXShaderCompiler instance
        struct DxcInstance {
            ComPtr<IDxcUtils> utils;
            ComPtr<IDxcCompiler3> compiler;
            ComPtr<IDxcIncludeHandler> include_handler;
        };

        // Create IDxcUtils, IDxcCompiler3 & IDxcIncludeHandler
        DxcInstance* dxc_create_instance()
        {
            ComPtr<IDxcUtils> utils;
            if (auto hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)); FAILED(hr)) {
                throw DxcInitError(hr);
            }
            SPDLOG_LOGGER_TRACE(ShaderCompiler::logger(), "Created IDxcUtils");
            ComPtr<IDxcCompiler3> compiler;
            if (auto hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)); FAILED(hr)) {
                throw DxcInitError(hr);
            }
            SPDLOG_LOGGER_TRACE(ShaderCompiler::logger(), "Created IDxcCompiler3");
            ComPtr<IDxcIncludeHandler> include_handler;
            if (auto hr = utils->CreateDefaultIncludeHandler(&include_handler); FAILED(hr)) {
                throw DxcInitError(hr);
            }
            SPDLOG_LOGGER_TRACE(ShaderCompiler::logger(), "Created IDxcIncludeHandler");
            SPDLOG_LOGGER_DEBUG(ShaderCompiler::logger(), "Created Dxc instance");
            return new DxcInstance{std::move(utils), std::move(compiler), std::move(include_handler)};
        }

        void dxc_destroy_instance(DxcInstance* instance)
        {
            delete instance;
        }
    } // namespace detail

    DxcInitError::DxcInitError(long hresult)
        : hresult_(hresult)
    {
    }

    DxcCompileError::DxcCompileError(ShaderCompileError compile_error, const char* msg)
        : compile_error_(compile_error)
        , msg_(msg)
    {
    }

    ShaderCompiler::ShaderCompiler()
        : dxc_instance_(detail::dxc_create_instance(), &detail::dxc_destroy_instance)
    {
    }

    // Internal helpers for shader compilation
    namespace
    {
        // Returns the DXC shader profile appropriate for the shader type
        const wchar_t* to_shader_profile(ShaderType shader_type) noexcept
        {
            switch (shader_type) {
                case ShaderType::Vertex:
                    return L"vs_6_0";
                case ShaderType::Fragment:
                    return L"ps_6_0";
            }
            ORION_ASSERT(!"Invalid shader type");
            return nullptr;
        }
    } // namespace

    // Compile HLSL source code with IDxcCompiler3::Compile() and return the results in a vector of bytes
    ShaderCompileResult ShaderCompiler::compile(const ShaderCompileDesc& compile_desc)
    {
        ORION_EXPECTS(!compile_desc.shader_source.empty());
        ORION_EXPECTS(compile_desc.entry_point != nullptr);

        // Create DxcBuffer to source string
        const DxcBuffer dxc_buffer{
            .Ptr = compile_desc.shader_source.data(),
            .Size = compile_desc.shader_source.size(),
            .Encoding = DXC_CP_ACP,
        };

        // Convert char strings to wchar strings
        auto to_wstring = [](const char* string) {
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4996)
#endif
            static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.from_bytes(string);
#ifdef _MSC_VER
    #pragma warning(pop)
#endif
        };

        // Entry point as wstring
        const auto entry_point = to_wstring(compile_desc.entry_point);

        // Arguments passed to dxc. Must be WSTRING
        static constexpr std::size_t max_arguments = 32;
        static_vector<const wchar_t*, max_arguments> arguments;
        arguments.push_back(L"-E");
        arguments.push_back(entry_point.c_str());
        arguments.push_back(L"-T");
        arguments.push_back(to_shader_profile(compile_desc.shader_type));
        if (compile_desc.object_type == ShaderObjectType::SpirV) {
            arguments.push_back(L"-spirv");
            if (compile_desc.shader_type == ShaderType::Vertex) {
                arguments.push_back(L"-fvk-invert-y");
            }
        }

        // Local refs to dxc objects
        auto compiler = dxc_instance_->compiler;
        auto include_handler = dxc_instance_->include_handler;

        // Throw internal error exception on hresult failure with on a function call
        auto hresult_check = [](HRESULT hr, const char* msg) {
            if (FAILED(hr)) {
                throw DxcCompileError(ShaderCompileError::InternalError, msg);
            }
        };

        // Compile and check for failed HRESULT. Additional error checking is performed after by checking the error buffer
        ComPtr<IDxcResult> results;
        hresult_check(compiler->Compile(&dxc_buffer, arguments.data(),
                                        static_cast<std::uint32_t>(arguments.size()),
                                        include_handler.Get(),
                                        IID_PPV_ARGS(&results)),
                      "Call to Compile() failed");

        // Check error buffer
        ComPtr<IDxcBlobUtf8> errors = nullptr;
        hresult_check(results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr), "GetOutput(DXC_OUT_ERRORS) failed");
        if (errors != nullptr && errors->GetStringLength() != 0) {
            SPDLOG_LOGGER_ERROR(logger(), "Shader compilation errors:");
            SPDLOG_LOGGER_ERROR(logger(), "{}", errors->GetStringPointer());
        }

        // Quit if compilation failed
        HRESULT compilation_status;
        hresult_check(results->GetStatus(&compilation_status), "GetStatus() failed");
        if (FAILED(compilation_status)) {
            SPDLOG_LOGGER_ERROR(logger(), "Shader compilation failed!");
            throw DxcCompileError(ShaderCompileError::CompilationFail, nullptr);
        }

        // The returned compile result
        ShaderCompileResult compile_result;

        // Get the object code
        ComPtr<IDxcBlob> shader_binary = nullptr;
        hresult_check(results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader_binary), nullptr), "GetOutput(DXC_OUT_OBJECT) failed");
        if (shader_binary != nullptr) {
            compile_result.binary.resize(shader_binary->GetBufferSize());
            std::memcpy(compile_result.binary.data(), shader_binary->GetBufferPointer(), shader_binary->GetBufferSize());
        }

        return compile_result;
    }

    spdlog::logger* ShaderCompiler::logger()
    {
        static const auto dxc_logger = []() {
            auto logger = spdlog::stdout_color_mt("orion-shader-compiler");
            logger->set_pattern("[%n] [%^%l%$] %v");
            logger->set_level(static_cast<spdlog::level::level_enum>(ORION_SHADER_COMPILER_LOG_LEVEL));
            return logger;
        }();
        return dxc_logger.get();
    }
} // namespace orion
