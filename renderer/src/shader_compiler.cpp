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
                SPDLOG_LOGGER_ERROR(logger(), "Failed to create IDxcUtils. HRESULT: {}", hr);
                return nullptr;
            }
            SPDLOG_LOGGER_TRACE(logger(), "Created IDxcUtils");
            ComPtr<IDxcCompiler3> compiler;
            if (auto hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)); FAILED(hr)) {
                SPDLOG_LOGGER_ERROR(logger(), "Failed to create IDxcCompiler3. HRESULT: {}", hr);
                return nullptr;
            }
            SPDLOG_LOGGER_TRACE(logger(), "Created IDxcCompiler3");
            ComPtr<IDxcIncludeHandler> include_handler;
            if (auto hr = utils->CreateDefaultIncludeHandler(&include_handler); FAILED(hr)) {
                SPDLOG_LOGGER_ERROR(logger(), "Failed to create IDxcIncludeHandler. HRESULT: {}", hr);
                return nullptr;
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

    ShaderCompiler::ShaderCompiler()
        : dxc_instance_(detail::dxc_create_instance(), &detail::dxc_destroy_instance)
    {
        ORION_ENSURES(dxc_instance_ != nullptr);
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
            ComPtr<IDxcResult> results;
            const auto compile_hresult = compiler->Compile(&dxc_buffer,
                                                           arguments.data(), static_cast<uint32_t>(arguments.size()),
                                                           include_handler.Get(), IID_PPV_ARGS(&results));
            if (FAILED(compile_hresult)) {
                SPDLOG_LOGGER_ERROR(logger(), "Call to IDxcCompiler3::Compile() failed. HRESULT: {}", compile_hresult);
                return make_unexpected(ShaderCompileError::InternalError);
            }

            // Check error buffer
            ComPtr<IDxcBlobUtf8> errors = nullptr;
            if (FAILED(results->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr))) {
                SPDLOG_LOGGER_ERROR(logger(), "Call to GetOutput(DXC_OUT_ERRORS) failed");
                return make_unexpected(ShaderCompileError::InternalError);
            }
            if (errors != nullptr && errors->GetStringLength() != 0) {
                SPDLOG_LOGGER_ERROR(logger(), "Shader compilation errors:");
                SPDLOG_LOGGER_ERROR(logger(), "{}", errors->GetStringPointer());
            }

            // Quit if compilation failed
            HRESULT compilation_status = S_OK;
            if (FAILED(results->GetStatus(&compilation_status))) {
                SPDLOG_LOGGER_ERROR(logger(), "Call to GetStatus() failed");
                return make_unexpected(ShaderCompileError::InternalError);
            }
            if (FAILED(compilation_status)) {
                SPDLOG_LOGGER_ERROR(logger(), "Shader compilation failed!");
                return make_unexpected(ShaderCompileError::CompilationFail);
            }

            // The returned compile result
            ShaderCompileSuccess compile_result;

            // Get the object code
            ComPtr<IDxcBlob> shader_binary = nullptr;
            if (FAILED(results->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader_binary), nullptr))) {
                SPDLOG_LOGGER_ERROR(logger(), "Call to GetOutput(DXC_OUT_OBJECT) failed");
                return make_unexpected(ShaderCompileError::InternalError);
            }
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
        if (FAILED(utils->CreateBlob(source.data(), static_cast<UINT32>(source.size()), 0, &source_blob))) {
            SPDLOG_LOGGER_ERROR(logger(), "Failed to create source blob from source string");
            return make_unexpected(ShaderCompileError::InvalidSource);
        }

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
        if (FAILED(utils->LoadFile(w_filename.c_str(), nullptr, &source_blob))) {
            SPDLOG_LOGGER_ERROR(logger(), "Failed to load shader source file '{}'", source_file);
            return make_unexpected(ShaderCompileError::FailedLoadFile);
        }

        return compile_blob(source_blob.Get(), desc, dxc_instance_.get());
    }
} // namespace orion
