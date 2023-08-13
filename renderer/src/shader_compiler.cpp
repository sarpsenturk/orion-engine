#include "orion-renderer/shader_compiler.h"

#define NOMINMAX
#define WIN32_LEAN_AND_MEA

#include "dxc_include.h"
#include "orion-utils/assertion.h"
#include "orion-utils/static_vector.h"

#include <codecvt>
#include <cstring>
#include <locale>
#include <span>

#ifndef ORION_SHADER_COMPILER_LOG_LEVEL
    #define ORION_SHADER_COMPILER_LOG_LEVEL SPDLOG_ACTIVE_LEVEL
#endif

#include "orion-core/log.h"
#include <spdlog/spdlog.h>

namespace orion
{
    namespace
    {
        spdlog::logger* logger()
        {
            static const auto dxc_logger = create_logger("orion-shader-compiler", ORION_SHADER_COMPILER_LOG_LEVEL);
            return dxc_logger.get();
        }
    } // namespace

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
            SPDLOG_LOGGER_TRACE(logger(), "Created IDxcUtils");
            ComPtr<IDxcCompiler3> compiler;
            if (auto hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)); FAILED(hr)) {
                throw DxcInitError(hr);
            }
            SPDLOG_LOGGER_TRACE(logger(), "Created IDxcCompiler3");
            ComPtr<IDxcIncludeHandler> include_handler;
            if (auto hr = utils->CreateDefaultIncludeHandler(&include_handler); FAILED(hr)) {
                throw DxcInitError(hr);
            }
            SPDLOG_LOGGER_TRACE(logger(), "Created IDxcIncludeHandler");
            SPDLOG_LOGGER_DEBUG(logger(), "Created Dxc instance");
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
        const wchar_t* to_shader_profile(ShaderStageFlags shader_type) noexcept
        {
            switch (shader_type) {
                case ShaderStageFlags::Vertex:
                    return L"vs_6_0";
                case ShaderStageFlags::Fragment:
                    return L"ps_6_0";
            }
            ORION_ASSERT(!"Invalid shader type");
            return nullptr;
        }

        void hresult_check(HRESULT hr, const char* msg)
        {
            if (FAILED(hr)) {
                throw DxcCompileError(ShaderCompileError::InternalError, msg);
            }
        }

        auto to_wstring(const char* string)
        {
#ifdef _MSC_VER
    #pragma warning(push)
    #pragma warning(disable : 4996)
#endif
            static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.from_bytes(string);
#ifdef _MSC_VER
    #pragma warning(pop)
#endif
        }

        ComPtr<IDxcResult> dxc_compile(IDxcCompiler3* compiler, IDxcIncludeHandler* include_handler, const DxcBuffer* buffer, std::span<LPCWSTR> arguments)
        {
            ComPtr<IDxcResult> results;
            hresult_check(compiler->Compile(
                              buffer,
                              arguments.data(), static_cast<uint32_t>(arguments.size()),
                              include_handler,
                              IID_PPV_ARGS(&results)),
                          "Call to Compile() failed");
            return results;
        }

        ShaderCompileResult compile_blob(IDxcBlobEncoding* source, const ShaderCompileDesc& desc, detail::DxcInstance* dxc)
        {
            // Create DxcBuffer to source string
            const DxcBuffer dxc_buffer{
                .Ptr = source->GetBufferPointer(),
                .Size = source->GetBufferSize(),
                .Encoding = DXC_CP_ACP,
            };

            // Entry point as wstring
            const auto entry_point = to_wstring(desc.entry_point);

            // Arguments passed to dxc. Must be WSTRING
            static constexpr std::size_t max_arguments = 32;
            static_vector<const wchar_t*, max_arguments> arguments;
            arguments.push_back(L"-E");
            arguments.push_back(entry_point.c_str());
            arguments.push_back(L"-T");
            arguments.push_back(to_shader_profile(desc.shader_stage));
            if (desc.object_type == ShaderObjectType::SpirV) {
                arguments.push_back(L"-spirv");
                if (desc.shader_stage == ShaderStageFlags::Vertex) {
                    arguments.push_back(L"-fvk-invert-y");
                }
            }

            // Local refs to dxc objects
            auto compiler = dxc->compiler;
            auto include_handler = dxc->include_handler;

            // Compile and check for failed HRESULT. Additional error checking is performed after by checking the error buffer
            const auto results = dxc_compile(compiler.Get(), include_handler.Get(), &dxc_buffer, arguments);

            // Check error buffer
            ComPtr<IDxcBlobUtf8> errors = nullptr;
            hresult_check(results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr), "GetOutput(DXC_OUT_ERRORS) failed");
            if (errors != nullptr && errors->GetStringLength() != 0) {
                SPDLOG_LOGGER_ERROR(logger(), "Shader compilation errors:");
                SPDLOG_LOGGER_ERROR(logger(), "{}", errors->GetStringPointer());
            }

            // Quit if compilation failed
            HRESULT compilation_status = S_OK;
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
    } // namespace

    ShaderCompileResult ShaderCompiler::compile_from_source(std::string_view source, const ShaderCompileDesc& desc) const
    {
        ORION_EXPECTS(!source.empty());

        // Get pointer to utils
        auto* utils = dxc_instance_->utils.Get();

        // Create blob from source string
        ComPtr<IDxcBlobEncoding> source_blob = nullptr;
        hresult_check(utils->CreateBlob(source.data(), static_cast<UINT32>(source.size()), 0, &source_blob), "Failed to create IDxcBlobEncoding");

        return compile_blob(source_blob.Get(), desc, dxc_instance_.get());
    }

    ShaderCompileResult ShaderCompiler::compile_from_file(const std::string& source_file, const ShaderCompileDesc& desc) const
    {
        ORION_EXPECTS(!source_file.empty());
        ComPtr<IDxcBlobEncoding> source_blob = nullptr;

        // Get pointer to utils
        auto* utils = dxc_instance_->utils.Get();

        // Create blob from file
        const auto w_filename = to_wstring(source_file.c_str());
        hresult_check(utils->LoadFile(w_filename.c_str(), nullptr, &source_blob), "Failed to load source file");

        return compile_blob(source_blob.Get(), desc, dxc_instance_.get());
    }
} // namespace orion
